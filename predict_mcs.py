import argparse
import numpy as np
import pandas as pd


def load_model(path):
    if path.endswith(".keras") or path.endswith(".h5"):
        from tensorflow.keras.models import load_model as klm
        return klm(path), True
    import joblib
    return joblib.load(path), False


def predict(model, x, is_keras):
    if is_keras:
        x3 = x.reshape((x.shape[0], 1, x.shape[1]))
        return model.predict(x3, verbose=0).reshape(-1)
    return model.predict(x)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", required=True)
    ap.add_argument("--coords", default="coords_50s.csv")
    ap.add_argument("--out", default="predicted_mcs.csv")
    args = ap.parse_args()

    model, is_keras = load_model(args.model)
    df = pd.read_csv(args.coords)
    df = df.dropna()

    x = df[["x", "y"]].values.astype("float32")
    pred = predict(model, x, is_keras)
    pred_round = np.clip(np.rint(pred), 0, 28).astype(int)

    time_ms = np.rint(df["time"].values * 1000.0).astype(np.int64)
    ue = df["ue"].values.astype(int)

    out = pd.DataFrame({"time_ms": time_ms, "ue": ue, "pred_mcs": pred_round})
    out.to_csv(args.out, index=False)
    print("Wrote", len(out), "predictions to", args.out)


if __name__ == "__main__":
    main()
