#!/usr/bin/env python3
"""
CityAirQ Parity Verification Tool

Validates that the serialized binary decision trees evaluate identical outputs
when traversed by our custom C++ algorithm as they do in Python scikit-learn/XGBoost.
"""

import os
import sys
import struct
import numpy as np
import pandas as pd
from xgboost import XGBRegressor

# Feature Enums mapping
FEATURE_MAPPING = [
    'FEATURE_AMBIENT_TEMP',
    'FEATURE_AMBIENT_RH',
    'FEATURE_GAS_RESISTANCE_A',
    'FEATURE_GAS_RESISTANCE_B',
    'FEATURE_GAS_RESISTANCE_C',
    'FEATURE_GAS_RESISTANCE_D',
    'FEATURE_PM1_0',
    'FEATURE_PM2_5',
    'FEATURE_PM10_0',
    'FEATURE_CO2',
    'FEATURE_BARO_PRESSURE'
]
ML_FEATURE_COUNT = 11

def parse_bin_model(filepath):
    """
    Parses our custom binary model and returns the ensemble elements.
    """
    with open(filepath, 'rb') as f:
        magic = f.read(4)
        if magic != b'MLCA':
            raise ValueError("Invalid magic header")
            
        base_score, tree_count, node_count = struct.unpack('<fHH', f.read(8))
        
        trees = []
        for _ in range(tree_count):
            offset, count = struct.unpack('<HH', f.read(4))
            trees.append({'offset': offset, 'count': count})
            
        nodes = []
        for _ in range(node_count):
            # Format: <bBhh2xff
            data = f.read(16)
            feature, reserved, left, right, threshold, value = struct.unpack('<bBhh2xff', data)
            nodes.append({
                'feature': feature,
                'left': left,
                'right': right,
                'threshold': threshold,
                'value': value
            })
            
    return base_score, trees, nodes

def evaluate_custom_tree(nodes, tree, features):
    """
    Mimics our C++ DecisionTreeEngine evaluateTree logic.
    """
    current = tree['offset']
    limit = tree['offset'] + tree['count']
    
    while current >= tree['offset'] and current < limit:
        node = nodes[current]
        if node['feature'] == -1:
            return node['value']
            
        if node['feature'] < 0 or node['feature'] >= ML_FEATURE_COUNT:
            return 0.0
            
        feat_val = features[node['feature']]
        if feat_val <= node['threshold']:
            current = node['left']
        else:
            current = node['right']
            
    return 0.0

def evaluate_custom_ensemble(base_score, trees, nodes, features):
    """
    Mimics our C++ DecisionTreeEngine evaluateEnsemble logic.
    """
    prediction = base_score
    for tree in trees:
        prediction += evaluate_custom_tree(nodes, tree, features)
    return prediction

def main():
    print("=== CityAirQ Inference Parity Verification Test ===")
    
    # 1. Create a dummy training dataset and train a mock model
    np.random.seed(42)
    sample_size = 500
    
    data = {}
    for feat in FEATURE_MAPPING:
        data[feat] = np.random.uniform(0.0, 1000.0, sample_size)
        
    X = pd.DataFrame(data)
    y = 0.5 * X['FEATURE_AMBIENT_TEMP'] - 0.2 * X['FEATURE_AMBIENT_RH'] + np.random.normal(0, 5, sample_size)
    
    model = XGBRegressor(n_estimators=10, max_depth=4, learning_rate=0.1, random_state=42)
    model.fit(X, y)
    
    # 2. Serialize model to binary using our script module
    sys.path.append(os.path.dirname(os.path.abspath(__file__)))
    from train_calib import serialize_tree_to_binary
    
    temp_bin_path = "./temp_parity_test_model.bin"
    serialize_tree_to_binary(model.get_booster(), temp_bin_path)
    
    # 3. Read back the binary model
    base_score, trees, nodes = parse_bin_model(temp_bin_path)
    
    # 4. Generate random test samples, run predictions through both XGBoost and custom C++ emulator
    test_samples = 100
    max_diff = 0.0
    passed_counts = 0
    
    for i in range(test_samples):
        # Create random input feature vector
        sample_features = np.random.uniform(-50.0, 2000.0, ML_FEATURE_COUNT).astype(np.float32)
        
        # XGBoost prediction
        sample_df = pd.DataFrame([sample_features], columns=FEATURE_MAPPING)
        xgboost_pred = float(model.predict(sample_df)[0])
        
        # Custom C++ traversal logic prediction
        custom_pred = evaluate_custom_ensemble(base_score, trees, nodes, sample_features)
        
        diff = abs(xgboost_pred - custom_pred)
        if diff > max_diff:
            max_diff = diff
            
        if diff < 1e-4:
            passed_counts += 1
        else:
            print(f"Sample {i} FAILED: XGBoost={xgboost_pred:.6f}, Custom={custom_pred:.6f}, Diff={diff:.6f}")

    # Cleanup temp model
    if os.path.exists(temp_bin_path):
        os.remove(temp_bin_path)
        
    print("\n=== Parity Verification Results ===")
    print(f"Total Test Samples Evaluated: {test_samples}")
    print(f"Successful matches (< 1e-4):  {passed_counts}")
    print(f"Max absolute discrepancy:     {max_diff:.8e}")
    
    if passed_counts == test_samples:
        print("\nSUCCESS: Mathematical parity verified to < 1e-4 absolute difference!")
        sys.exit(0)
    else:
        print("\nFAILURE: Outputs did not match. Traversal math mismatch.")
        sys.exit(1)


if __name__ == "__main__":
    main()
