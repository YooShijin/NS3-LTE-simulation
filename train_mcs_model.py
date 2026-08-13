import argparse
import os
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_absolute_error


def load_data(path):
    df = pd.read_csv(path)
    df = df.dropna()
    df = df[df["mcs"] >= 0]
    x = df[["x", "y"]].values.astype("float32")
    y = df["mcs"].values.astype("float32")
    return x, y


def train_random_forest(xtr, ytr):
    from sklearn.ensemble import RandomForestRegressor
    model = RandomForestRegressor(n_estimators=200, random_state=42, n_jobs=-1)
    model.fit(xtr, ytr)
    return model


def train_xgboost(xtr, ytr):
    from xgboost import XGBRegressor
    model = XGBRegressor(n_estimators=300, max_depth=6, learning_rate=0.1,
                         subsample=0.9, random_state=42)
    model.fit(xtr, ytr)
    return model


def build_keras(kind, timesteps, features):
    from tensorflow.keras.models import Sequential
    from tensorflow.keras.layers import Dense, LSTM, Bidirectional, SimpleRNN
    model = Sequential()
    if kind == "lstm":
        model.add(LSTM(64, input_shape=(timesteps, features)))
    elif kind == "bilstm":
        model.add(Bidirectional(LSTM(64), input_shape=(timesteps, features)))
    elif kind == "rnn":
        model.add(SimpleRNN(64, input_shape=(timesteps, features)))
    model.add(Dense(32, activation="relu"))
    model.add(Dense(1))
    model.compile(optimizer="adam", loss="mse")
    return model


def train_keras(kind, xtr, ytr):
    xtr3 = xtr.reshape((xtr.shape[0], 1, xtr.shape[1]))
    model = build_keras(kind, 1, xtr.shape[1])
    model.fit(xtr3, ytr, epochs=40, batch_size=64, verbose=0)
    return model


def predict(model, x, is_keras):
    if is_keras:
        x3 = x.reshape((x.shape[0], 1, x.shape[1]))
        return model.predict(x3, verbose=0).reshape(-1)
    return model.predict(x)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--data", default="mcs_dataset.csv")
    ap.add_argument("--model", default="random_forest",
                    choices=["random_forest", "lstm", "bilstm", "rnn", "xgboost"])
    ap.add_argument("--out", default="mcs_model")
    ap.add_argument("--plot", default="test_results.png")
    args = ap.parse_args()

    x, y = load_data(args.data)
    xtr, xte, ytr, yte = train_test_split(x, y, test_size=0.10, random_state=42)

    is_keras = args.model in ("lstm", "bilstm", "rnn")

    if args.model == "random_forest":
        model = train_random_forest(xtr, ytr)
    elif args.model == "xgboost":
        model = train_xgboost(xtr, ytr)
    else:
        model = train_keras(args.model, xtr, ytr)

    pred = predict(model, xte, is_keras)
    pred_round = np.clip(np.rint(pred), 0, 28).astype(int)
    yte_int = yte.astype(int)

    mae = mean_absolute_error(yte, pred)
    acc = float(np.mean(pred_round == yte_int))
    within1 = float(np.mean(np.abs(pred_round - yte_int) <= 1))
    print("Model:", args.model)
    print("Test samples:", len(yte))
    print("MAE:", round(mae, 3))
    print("Exact match accuracy:", round(acc, 3))
    print("Within +/-1 accuracy:", round(within1, 3))

    order = np.argsort(yte_int)
    plt.figure(figsize=(11, 5))
    plt.subplot(1, 2, 1)
    n = min(300, len(yte))
    plt.plot(yte_int[order][:n], label="Actual MCS", linewidth=1.2)
    plt.plot(pred_round[order][:n], label="Predicted MCS", linewidth=1.0, alpha=0.8)
    plt.xlabel("Test sample (sorted by actual MCS)")
    plt.ylabel("MCS")
    plt.title(args.model + " actual vs predicted")
    plt.legend()

    plt.subplot(1, 2, 2)
    plt.scatter(yte_int, pred_round, s=6, alpha=0.4)
    plt.plot([0, 28], [0, 28], color="red", linewidth=1)
    plt.xlabel("Actual MCS")
    plt.ylabel("Predicted MCS")
    plt.title("Test scatter")
    plt.tight_layout()
    plt.savefig(args.plot, dpi=140)
    print("Saved plot:", args.plot)

    if is_keras:
        model.save(args.out + ".keras")
        print("Saved model:", args.out + ".keras")
    else:
        import joblib
        joblib.dump(model, args.out + ".joblib")
        print("Saved model:", args.out + ".joblib")


if __name__ == "__main__":
    main()
