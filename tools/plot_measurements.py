#!/usr/bin/env python3
"""Chioccia — plots the CSV files written by chioccia_measure (requires matplotlib).

Usage: python3 tools/plot_measurements.py docs/misure
"""
import csv
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402


def read(path):
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def static_curves(d: Path):
    rows = read(d / "static_curves.csv")
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.5), sharey=True)
    for ax, mode in zip(axes, ("compress", "limit")):
        series = defaultdict(list)
        for r in rows:
            if r["mode"] == mode:
                series[float(r["pr"])].append((float(r["in_dbfs_rms"]), float(r["out_dbfs_rms"])))
        for pr, pts in sorted(series.items()):
            pts.sort()
            ax.plot([p[0] for p in pts], [p[1] for p in pts], label=f"PR {pr:g}")
        ax.plot([-40, 0], [-40, 0], "k:", lw=0.8, label="1:1")
        ax.set_title(f"Curva statica — {mode.capitalize()} (1 kHz, Gain 50)")
        ax.set_xlabel("Ingresso (dBFS RMS)")
        ax.grid(alpha=0.3)
        ax.legend(fontsize=8)
    axes[0].set_ylabel("Uscita (dBFS RMS)")
    fig.tight_layout()
    fig.savefig(d / "static_curves.png", dpi=110)


def attack(d: Path):
    rows = read(d / "attack.csv")
    fig, ax = plt.subplots(figsize=(8, 4))
    series = defaultdict(list)
    for r in rows:
        series[(r["mode"], r["level"])].append((float(r["t"]) * 1000.0, float(r["gr"])))
    for (mode, lvl), pts in sorted(series.items()):
        ax.plot([p[0] for p in pts], [p[1] for p in pts], ls="-" if mode == "compress" else "--", label=f"{mode} {lvl} dBFS")
    ax.set_xlabel("Tempo dal gradino (ms)")
    ax.set_ylabel("Riduzione di guadagno (dB)")
    ax.set_title("Attacco — PR 60, gradino da −40 dBFS")
    ax.grid(alpha=0.3)
    ax.legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(d / "attack.png", dpi=110)


def release(d: Path):
    rows = read(d / "release.csv")
    fig, ax = plt.subplots(figsize=(8, 4))
    series = defaultdict(list)
    for r in rows:
        series[float(r["burst_s"])].append((float(r["t"]), float(r["gr"])))
    for burst, pts in sorted(series.items()):
        ax.plot([p[0] for p in pts], [p[1] for p in pts], label=f"burst {burst:g} s")
    ax.set_xlabel("Tempo dalla fine del burst (s)")
    ax.set_ylabel("Riduzione di guadagno (dB)")
    ax.set_title("Rilascio in funzione della durata della compressione precedente (memoria)")
    ax.grid(alpha=0.3)
    ax.legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(d / "release.png", dpi=110)


def thd(d: Path):
    rows = read(d / "thd.csv")
    fig, axes = plt.subplots(1, 3, figsize=(13, 4), sharey=True)
    for ax, freq in zip(axes, ("1000", "100", "50")):
        series = defaultdict(list)
        for r in rows:
            if r["freq"] == freq and float(r["colour"]) > 0:
                series[float(r["colour"])].append((float(r["level_dbfs_peak"]), max(float(r["thd_percent"]), 1e-4)))
        for colour, pts in sorted(series.items()):
            pts.sort()
            ax.semilogy([p[0] for p in pts], [p[1] for p in pts], marker="o", label=f"Colour {colour:g} %")
        ax.set_title(f"THD a {freq} Hz (PR 0)")
        ax.set_xlabel("Livello (dBFS picco)")
        ax.grid(alpha=0.3, which="both")
        ax.legend(fontsize=8)
    axes[0].set_ylabel("THD (%)")
    fig.tight_layout()
    fig.savefig(d / "thd.png", dpi=110)


if __name__ == "__main__":
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "docs/misure")
    static_curves(out)
    attack(out)
    release(out)
    thd(out)
    print("plots written to", out)
