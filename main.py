import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split, KFold
from sklearn.preprocessing import StandardScaler, PolynomialFeatures
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score

np.random.seed(42)
torch.manual_seed(42)

class Softmax(nn.Module):
    """
    Softmax implementation that defines parameter
    matrices explicitly and calculates output via matmul
    """
    def __init__(self, n_features, n_classes):
        super(Softmax, self).__init__()
        self.W = nn.Parameter(torch.zeros(n_features, n_classes, dtype=torch.float32))
        self.b = nn.Parameter(torch.zeros(n_classes, dtype=torch.float32))
        
        nn.init.xavier_uniform_(self.W)

    def forward(self, X):
        return torch.matmul(X, self.W) + self.b

def main():
    iris = load_iris()
    X = iris.data
    y = iris.target
    n_classes = len(np.unique(y))

    X_train_full, X_test, y_train_full, y_test = train_test_split(X, y, test_size=0.15, random_state=42)
    
    # 70 / 15 / 15 split - train/val/test
    val_size_ratio = 0.15 / 0.85
    X_train, X_val, y_train, y_val = train_test_split(X_train_full, y_train_full, test_size=val_size_ratio, random_state=42)

    print(f"split: train={len(X_train)} val={len(X_val)} test={len(X_test)}")

    # k-fold validation
    kf = KFold(n_splits=3, shuffle=True, random_state=42)
    degrees = [1, 2, 3]
    avg_val_losses = []

    for degree in degrees:
        poly = PolynomialFeatures(degree=degree, include_bias=False)
        X_poly_train = poly.fit_transform(X_train)        
        val_losses_fold = []
        
        for train_idx, val_idx in kf.split(X_poly_train):
            X_fold_train, X_fold_val = X_poly_train[train_idx], X_poly_train[val_idx]
            y_fold_train, y_fold_val = y_train[train_idx], y_train[val_idx]
            
            scaler = StandardScaler()
            X_fold_train_scaled = scaler.fit_transform(X_fold_train)
            X_fold_val_scaled = scaler.transform(X_fold_val)
            
            X_tr_t = torch.tensor(X_fold_train_scaled, dtype=torch.float32)
            y_tr_t = torch.tensor(y_fold_train, dtype=torch.long)
            X_vl_t = torch.tensor(X_fold_val_scaled, dtype=torch.float32)
            y_vl_t = torch.tensor(y_fold_val, dtype=torch.long)
            
            model = Softmax(n_features=X_fold_train.shape[1], n_classes=n_classes)
            criterion = nn.CrossEntropyLoss()
            optimizer = optim.Adam(model.parameters(), lr=0.01)
            
            for epoch in range(100):
                model.train()
                optimizer.zero_grad()
                loss = criterion(model(X_tr_t), y_tr_t)
                loss.backward()
                optimizer.step()
                
            model.eval()
            with torch.no_grad():
                val_losses_fold.append(criterion(model(X_vl_t), y_vl_t).item())
                
        avg_loss = np.mean(val_losses_fold)
        avg_val_losses.append((degree, avg_loss))
        print(f"degree {degree} cv loss: {avg_loss:.4f}")

    best_degree = sorted(avg_val_losses, key=lambda x: x[1])[0][0]
    print(f"best degree: {best_degree}")

    poly = PolynomialFeatures(degree=best_degree, include_bias=False)
    X_train_poly = poly.fit_transform(X_train)
    X_val_poly = poly.transform(X_val)
    X_test_poly = poly.transform(X_test)

    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train_poly)
    X_val_scaled = scaler.transform(X_val_poly)
    X_test_scaled = scaler.transform(X_test_poly)

    X_train_t = torch.tensor(X_train_scaled, dtype=torch.float32)
    y_train_t = torch.tensor(y_train, dtype=torch.long)
    X_val_t = torch.tensor(X_val_scaled, dtype=torch.float32)
    y_val_t = torch.tensor(y_val, dtype=torch.long)
    X_test_t = torch.tensor(X_test_scaled, dtype=torch.float32)
    y_test_t = torch.tensor(y_test, dtype=torch.long)

    l1, l2, l3 = 0.000015, 0.0015, 0.15
    learning_rates = [l1, l2, l3]
    learning_rate_names = ['l1 (1.5e-5)', 'l2 (1.5e-3)', 'l3 (0.15)']
    regularizations = ['Ridge', 'Lasso', 'ElasticNet']
    lambda_reg = 0.01

    history = {'train_loss': {}, 'val_loss': {}, 'test_loss': {}}
    best_val_error = float('inf')
    best_model_info = None

    print("\ntraining models..")
    for lr, lr_name in zip(learning_rates, learning_rate_names):
        for reg in regularizations:
            model_name = f"{reg}_{lr_name}"
            
            model = Softmax(n_features=X_train_scaled.shape[1], n_classes=n_classes)
            criterion = nn.CrossEntropyLoss()
            optimizer = optim.SGD(model.parameters(), lr=lr)
            
            train_curve, val_curve, test_curve = [], [], []
            
            for epoch in range(50):
                model.train()
                optimizer.zero_grad()
                base_loss = criterion(model(X_train_t), y_train_t)
                
                # apply manual penalties lasso/ridge/elasticnet
                if reg == 'Ridge':
                    loss = base_loss + lambda_reg * torch.sum(model.W ** 2)
                elif reg == 'Lasso':
                    loss = base_loss + lambda_reg * torch.sum(torch.abs(model.W))
                elif reg == 'ElasticNet':
                    loss = base_loss + lambda_reg * (0.5 * torch.sum(torch.abs(model.W)) + 0.5 * torch.sum(model.W ** 2))
                    
                loss.backward()
                optimizer.step()
                
                model.eval()
                with torch.no_grad():
                    train_curve.append(criterion(model(X_train_t), y_train_t).item())
                    val_curve.append(criterion(model(X_val_t), y_val_t).item())
                    test_curve.append(criterion(model(X_test_t), y_test_t).item())
                
            history['train_loss'][model_name] = train_curve
            history['val_loss'][model_name] = val_curve
            history['test_loss'][model_name] = test_curve
            
            final_val = val_curve[-1]
            if final_val < best_val_error:
                best_val_error = final_val
                
                model.eval()
                with torch.no_grad():
                    _, preds = torch.max(model(X_test_t), 1)
                    p_np = preds.numpy()
                    y_np = y_test_t.numpy()
                    
                    best_model_info = {
                        'name': model_name,
                        'acc': accuracy_score(y_np, p_np),
                        'prec': precision_score(y_np, p_np, average='macro', zero_division=0),
                        'rec': recall_score(y_np, p_np, average='macro', zero_division=0),
                        'f1': f1_score(y_np, p_np, average='macro', zero_division=0),
                        'val_loss': final_val
                    }

    print(f"\nbest: {best_model_info['name']} (val loss: {best_model_info['val_loss']:.4f})")
    print(f"metrics: acc={best_model_info['acc']:.2f} prec={best_model_info['prec']:.2f} rec={best_model_info['rec']:.2f} f1={best_model_info['f1']:.2f}")

    curve_titles = {
        'train_loss': 'Train Loss vs Epoch',
        'val_loss': 'Validation Loss vs Epoch',
        'test_loss': 'Test Loss vs Epoch',
    }
    curves = ['train_loss', 'val_loss', 'test_loss']
    for curve_key in curves:
        plt.figure(figsize=(10, 6))
        for m_name, vals in history[curve_key].items():
            plt.plot(vals, label=m_name)
        
        plt.title(curve_titles[curve_key])
        plt.xlabel('Epoch')
        plt.ylabel('Loss (Cross Entropy)')
        plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(f"{curve_key}.png")
        plt.close()

    print("saved plots.")

if __name__ == "__main__":
    main()
