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
#include "function.hpp"
#define PYTHON_EXE_PATH u8"Path to python" 
using namespace std;


void file_out(string &filename, vector<ele_unit> &element) {
    stringstream txtfilename, csvfilename, out_csv, out_det;
    txtfilename << "result_" << filename << ".txt";
    csvfilename << "result_" << filename << ".csv";
    ofstream fpcsv(csvfilename.str());
    ofstream fpout(txtfilename.str());
    ofstream fpresult("result.csv");
    string name;

    for (size_t i = 0; i < element.size(); i++) {
        name = element[i].element;
        if(element[i].ide_num == 2 ||element[i].ide_num == 3){
            name.replace(0,1,"J");
        }
        out_csv.clear();
        out_csv.str("");
        out_csv << setw(5) << name << "," << setw(6) << setprecision(4) << element[i].margin_L << "," << setw(6) << setprecision(4) << element[i].margin_H;
        fpcsv << out_csv.str() << endl;
        fpresult << out_csv.str() << endl;
    }

    for (size_t i = 0; i < element.size(); i++) {
        name = element[i].element;
        if(element[i].ide_num == 2 ||element[i].ide_num == 3){
            name.replace(0,1,"J");
        }
        out_det.clear();
        out_det.str("");
        out_det << setw(6) << right << name << " : " << fixed  << right << setw(6) << setprecision(3) << element[i].value  << " " << setw(5) << left <<  element[i].unit << " Lower: "  << setprecision(3) << setw(6) << element[i].value * (1 + element[i].margin_L / 100)  << "   Upper: "
                << setw(6) << setprecision(3)  << element[i].value * (1 + element[i].margin_H / 100) << "   Median: " << setw(6) << setprecision(3)  << element[i].value * (1 + (element[i].margin_L / 100 + element[i].margin_H / 100) / 2 ) << "  "  << setprecision(2) << setw(7) << element[i].margin_L << " % ~ "  << setprecision(2) << setw(6) << element[i].margin_H << " %";
        fpout << out_det.str() << endl;
    }

    fpcsv.close();
    fpout.close();
}

