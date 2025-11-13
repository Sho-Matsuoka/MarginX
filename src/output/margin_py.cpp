#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <cstdio>
#include <regex>
#include <algorithm>
#include <filesystem>
#include <cmath>
#include <math.h>
#include <iomanip> 
#include <cstdio>
#include <cstdlib>
#include "function.hpp"
using namespace std;



int margin_py(){
   // 1) Python スクリプトを文字列リテラルで用意
    const char *pycode = R"PYCODE(
import pandas as pd
import matplotlib.pyplot as plt

# CSV 読み込み
df = pd.read_csv("result.csv", header=None, names=["Label","Negative","Positive"])

# 描画
fig, ax = plt.subplots(figsize=(10,8))
df.plot.barh(
    x="Label", y=["Negative","Positive"],
    stacked=True, ax=ax, color="darkorange",
    legend=False, fontsize=15
)
plt.xlabel("Margin [%]", fontsize=15)
plt.ylabel("")
plt.xlim(-50,50)
plt.xticks([-50,-40,-30,-20,-10,0,10,20,30,40,50], fontsize=15)
plt.vlines(0.01, -1000, 1000, colors="darkgrey", linestyles="solid", linewidth=1)
ax.invert_yaxis()
plt.show()
)PYCODE";

    // 2) Python インタプリタを popen で起動してスクリプトを渡す
    FILE *pipe = popen("python3 -u", "w");
    if (!pipe) {
        perror("popen");
        return 1;
    }
    fprintf(pipe, "%s", pycode);
    pclose(pipe);

    system("rm -rf result.csv");

    return 0;
}
