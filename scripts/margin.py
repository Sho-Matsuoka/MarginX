# -*- coding: utf-8 -*-
import pandas as pd
import matplotlib.pyplot as plt
#import pprint
#import matplotlib.font_manager as fm

# CSVファイルを読み込む
data = pd.read_csv("result.csv", header=None, names=["Label", "Negative", "Positive"])
# グラフの描画
fig, ax = plt.subplots(figsize=(10, 8))
# データを積み上げ棒グラフで表示
data.plot.barh(x="Label", y=["Negative", "Positive"], stacked=True, ax=ax, color = "darkorange", legend = False, fontsize = 15)

#plt.rcParams["font.family"] ="Segoe UI"

# グラフのタイトルや軸ラベルの設定
plt.xlabel("Margin [%]", fontsize=15)

#Y軸のラベルを削除
plt.ylabel("")

#X軸の範囲を設定
plt.xlim(-50, 50)

#X軸の値を指定
plt.xticks([-50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50], fontsize = 15)

#0 % のラインを描画
plt.vlines(0.01,-1000, 1000, colors="darkgrey", linestyles='solid', label='', linewidth=1)
plt.tick_params(labelsize=15)
#デフォルトだと逆順で出力されるので、y軸を反転
ax.invert_yaxis()

# グラフを保存
#plt.savefig("result.png")
#font_list = [f.name for f in fm.fontManager.ttflist]

#pprint.pprint(font_list)

# グラフを表示
plt.show()
