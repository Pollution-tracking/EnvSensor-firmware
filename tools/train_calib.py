#!/usr/bin/env python3
"""
CityAirQ Model Calibration and Serialization Tool

Guides the user to train compact XGBoost models on collocated sensor
and reference data, exporting the results into binary calibration tree arrays
usable directly by the ESP32-S3 firmware.
"""

import os
import sys
import struct
import argparse
import numpy as np
import pandas as pd

try:
    from xgboost import XGBRegressor
    from sklearn.model_selection import train_test_split
    from sklearn.metrics import r2_score, root_mean_squared_error
except ImportError:
    print("Error: Missing required Python dependencies.")
    print("Please install them using: pip install xgboost scikit-learn pandas numpy")
    sys.exit(1)

# Feature Enums mapping from compensation_constants.h
FEATURE_MAPPING = {
    'FEATURE_AMBIENT_TEMP': 0,
    'FEATURE_AMBIENT_RH': 1,
    'FEATURE_GAS_RESISTANCE_A': 2,
    'FEATURE_GAS_RESISTANCE_B': 3,
    'FEATURE_GAS_RESISTANCE_C': 4,
    'FEATURE_GAS_RESISTANCE_D': 5,
    'FEATURE_PM1_0': 6,
    'FEATURE_PM2_5': 7,
    'FEATURE_PM10_0': 8,
    'FEATURE_CO2': 9,
    'FEATURE_BARO_PRESSURE': 10
}
ML_FEATURE_COUNT = 11

def parse_args():
    parser = argparse.ArgumentParser(description="CityAirQ XGBoost Model Calibration Trainer")
    parser.add_argument("--trace-csv", type=str, help="Path to raw/compensated SD log trace CSV file")
    parser.add_argument("--ref-csv", type=str, help="Path to reference station target CSV file")
    parser.add_argument("--target", type=str, choices=["pm1", "pm25", "pm10", "co", "no2", "nh3", "co2"],
                        help="Target pollutant calibration model to train")
    parser.add_argument("--n-estimators", type=int, default=10, help="Number of boosted trees (default: 10)")
    parser.add_argument("--max-depth", type=int, default=4, help="Maximum tree depth (default: 4)")
    parser.add_argument("--output-dir", type=str, default="./models", help="Directory to save the binary models")
    return parser.parse_args()

def serialize_tree_to_binary(booster, output_path):
    """
    Serializes an XGBoost model (JSON representation) into the binary MLCA format.
    Format specifications:
      - Magic: 'MLCA' (4 bytes)
      - baseScore: float (4 bytes)
      - treeCount: uint16_t (2 bytes)
      - nodeCount: uint16_t (2 bytes)
      - Tree offsets table: treeCount elements of (uint16_t offset, uint16_t count)
      - Node pool: nodeCount elements of 16-byte packed Node structure
    """
    df = booster.trees_to_dataframe()
    trees = []
    nodes = []
    
    # Group by tree ID
    tree_ids = sorted(df['Tree'].unique())
    node_offset = 0
    
    for tree_id in tree_ids:
        tree_df = df[df['Tree'] == tree_id].sort_values(by='Node')
        node_count = len(tree_df)
        trees.append((node_offset, node_count))
        
        # Build node maps for child indexing
        node_id_map = {row['Node']: idx for idx, (_, row) in enumerate(tree_df.iterrows())}
        
        for _, row in tree_df.iterrows():
            feature_name = row['Feature']
            node_type = row['Feature']
            
            if feature_name == 'Leaf':
                # Leaf Node
                feature_idx = -1
                left_child = 0
                right_child = 0
                threshold = 0.0
                value = float(row['Gain'])
            else:
                # Split Node
                feature_idx = FEATURE_MAPPING.get(feature_name, -1)
                if feature_idx == -1:
                    print(f"Warning: Unknown feature name '{feature_name}'. Skipping.")
                
                # Fetch absolute indices in the global node pool
                left_id = int(row['Yes'].split('-')[1])
                right_id = int(row['No'].split('-')[1])
                
                left_child = node_offset + node_id_map[left_id]
                right_child = node_offset + node_id_map[right_id]
                threshold = float(row['Split'])
                value = 0.0
            
            nodes.append({
                'feature': feature_idx,
                'left': left_child,
                'right': right_child,
                'threshold': threshold,
                'value': value
            })
            
        node_offset += node_count

    # Fetch base score (default prediction bias)
    try:
        import json
        config = json.loads(booster.save_config())
        base_score_str = config['learner']['learner_model_param']['base_score']
        base_score_str = base_score_str.strip('[]')
        base_score = float(base_score_str)
    except Exception:
        base_score = 0.0


    # Write binary file
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'wb') as f:
        # 1. Header
        f.write(b'MLCA')
        f.write(struct.pack('<fHH', base_score, len(trees), len(nodes)))
        
        # 2. Tree index table
        for offset, count in trees:
            f.write(struct.pack('<HH', offset, count))
            
        # 3. TreeNode pool (16-byte packed structs)
        # Struct format: <bBhh2xff
        #   - b (int8_t feature)
        #   - B (uint8_t reserved/padding)
        #   - h (int16_t left)
        #   - h (int16_t right)
        #   - 2x (2 bytes padding)
        #   - f (float threshold)
        #   - f (float value)
        for node in nodes:
            f.write(struct.pack(
                '<bBhh2xff',
                node['feature'],
                0, # reserved
                node['left'],
                node['right'],
                node['threshold'],
                node['value']
            ))

    print(f"Model successfully serialized to: {output_path}")
    print(f"  Binary size: {os.path.getsize(output_path)} bytes")

def main():
    args = parse_args()
    
    print("=== CityAirQ Model Calibration Trainer ===")
    
    # Interactive parameter fallback if arguments are missing
    trace_csv = args.trace_csv
    while not trace_csv or not os.path.exists(trace_csv):
        trace_csv = input("Enter path to your raw/compensated SD log trace CSV: ").strip()
        if not os.path.exists(trace_csv):
            print("File not found. Please try again.")
            
    ref_csv = args.ref_csv
    while not ref_csv or not os.path.exists(ref_csv):
        ref_csv = input("Enter path to the reference station data CSV: ").strip()
        if not os.path.exists(ref_csv):
            print("File not found. Please try again.")

    target = args.target
    while not target:
        target = input("Choose target calibration parameter (pm1, pm25, pm10, co, no2, nh3, co2): ").strip().lower()
        if target not in ["pm1", "pm25", "pm10", "co", "no2", "nh3", "co2"]:
            print("Invalid target chosen. Try again.")
            target = None

    print(f"\nLoading datasets...")
    df_trace = pd.read_csv(trace_csv)
    df_ref = pd.read_csv(ref_csv)
    
    # Standardize timestamp headers to match
    # Expecting columns: 'timestamp', or 'DateTime'
    for df in [df_trace, df_ref]:
        if 'DateTime' in df.columns:
            df.rename(columns={'DateTime': 'timestamp'}, inplace=True)
            
    if 'timestamp' not in df_trace.columns or 'timestamp' not in df_ref.columns:
        print("Error: Both CSVs must contain a 'timestamp' column for alignment.")
        sys.exit(1)
        
    df_trace['timestamp'] = pd.to_datetime(df_trace['timestamp'])
    df_ref['timestamp'] = pd.to_datetime(df_ref['timestamp'])
    
    # Merge on timestamp
    merged = pd.merge_asof(
        df_trace.sort_values('timestamp'),
        df_ref.sort_values('timestamp'),
        on='timestamp',
        direction='nearest',
        tolerance=pd.Timedelta('5min') # Allow up to 5-minute alignment tolerance
    )
    
    merged.dropna(subset=['ref_val' if 'ref_val' in merged.columns else merged.columns[-1]], inplace=True)
    if merged.empty:
        print("Error: No aligned data points found. Check timestamps or timezone alignments.")
        sys.exit(1)
        
    print(f"Aligned {len(merged)} datapoints for training.")

    # Populate features
    # Map features dynamically based on our MLFeature enum
    X = pd.DataFrame()
    X['FEATURE_AMBIENT_TEMP'] = merged['temp'] if 'temp' in merged.columns else 20.0
    X['FEATURE_AMBIENT_RH'] = merged['rh'] if 'rh' in merged.columns else 50.0
    
    if 'gas_bme' in merged.columns:
        X['FEATURE_GAS_RESISTANCE_A'] = merged['gas_bme']
    else:
        X['FEATURE_GAS_RESISTANCE_A'] = 0.0
        
    if 'mics_co' in merged.columns:
        X['FEATURE_GAS_RESISTANCE_B'] = merged['mics_co']
        X['FEATURE_GAS_RESISTANCE_C'] = merged['mics_no2']
        X['FEATURE_GAS_RESISTANCE_D'] = merged['mics_nh3']
    else:
        X['FEATURE_GAS_RESISTANCE_B'] = 0.0
        X['FEATURE_GAS_RESISTANCE_C'] = 0.0
        X['FEATURE_GAS_RESISTANCE_D'] = 0.0
        
    X['FEATURE_PM1_0'] = merged['pm1'] if 'pm1' in merged.columns else 0.0
    X['FEATURE_PM2_5'] = merged['pm25'] if 'pm25' in merged.columns else 0.0
    X['FEATURE_PM10_0'] = merged['pm10'] if 'pm10' in merged.columns else 0.0
    X['FEATURE_CO2'] = merged['co2'] if 'co2' in merged.columns else 400.0
    X['FEATURE_BARO_PRESSURE'] = merged['pressure'] if 'pressure' in merged.columns else 1013.25

    # Target: The residual difference between Reference value and our physical base value
    # If target is pm25, ref_pm25 - base_pm25
    ref_col = f'ref_{target}' if f'ref_{target}' in merged.columns else merged.columns[-1]
    
    base_col = f'comp_{target}' if f'comp_{target}' in merged.columns else target
    if base_col not in merged.columns:
        # Try raw value
        base_col = target
        
    y = merged[ref_col] - merged[base_col]
    
    # Train / Test split
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    
    print(f"\nTraining XGBoost Regressor model...")
    print(f"  n_estimators = {args.n_estimators}")
    print(f"  max_depth    = {args.max_depth}")
    
    model = XGBRegressor(
        n_estimators=args.n_estimators,
        max_depth=args.max_depth,
        learning_rate=0.1,
        random_state=42
    )
    model.fit(X_train, y_train)
    
    # Evaluate
    preds = model.predict(X_test)
    rmse = root_mean_squared_error(y_test, preds)
    r2 = r2_score(y_test, preds)
    
    print("\n=== Model Validation Evaluation ===")
    print(f"  Test RMSE (Residual Error): {rmse:.4f}")
    print(f"  Test R2 Score:              {r2:.4f}")
    
    # Save binary output
    output_filename = f"{target}_calib.bin"
    output_path = os.path.join(args.output_dir, output_filename)
    serialize_tree_to_binary(model.get_booster(), output_path)
    
    print("\nCopy the generated binary file to your SD card under '/models/' directory.")
    print("Boot the device to automatically apply your new machine learning calibration!")

if __name__ == "__main__":
    main()
