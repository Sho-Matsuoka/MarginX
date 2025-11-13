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

using namespace std;

void make_data_cir(vector<string> &data_cir, string &filename, vector<ele_unit> &element){
    
    int flg_shunt_B = 0;
    int line_num = 0;
    int i = 0;
    size_t i1 = 0, i2 = 0;
    int file_check = -1;
    range range;

    string cirfilename = filename + ".cir";
    string inpfilename = filename + ".inp";

    filename = cirfilename;

    if(!filesystem::is_regular_file(cirfilename)){ //if there is [input].cir, change the name of filename to .inp
        cirfilename = inpfilename;
        filename = inpfilename;
    }

    ifstream file(cirfilename);
    string line, name, node1, node2, ignore, jjmod, shunt, main, sub, unit;
    double value;
    stringstream liness, mainss;

    if (!file.is_open()) {
        cerr << "Error opening file2: " << cirfilename << endl;
    }
    while( getline(file, line)) {
            liness.clear();
            liness.str("");
            mainss.clear();
            mainss.str("");
            sub = "";
            unit = "";
            data_cir.emplace_back(line); // insert line to data_cir

            if (flg_shunt_B == 1){
                if(line.find("RS") == 0 || line.find("Rs") == 0){
                    i1 = line.rfind("=");
                    if (i1 != string::npos){
                        line.insert(i1 + 1, " ");
                    }
                    stringstream liness(line);
                    liness >> ignore >> ignore >> ignore >> ignore >> shunt >> value;

                    element[element.size() - 1].shunt_cal = value;  //shant_cal(shunt抵抗計算用)に値を格納

                    if(line.find("*SHUNT") != string::npos || line.find("*shunt") != string::npos){
                        element[element.size() - 1].shunt_det = 0;   //shunt_detの値を0(IcRsでシャント抵抗計算)に変更
                    }
                    else if(line.find("*Bc") != string::npos || line.find("*BC") != string::npos){
                        element[element.size() - 1].shunt_det = 1; //shunt_detの値を1(Bcでシャント抵抗計算)に変更
                    }
                    else if(line.find("*calc") != string::npos || line.find("*Calc") != string::npos || line.find("*CALC") != string::npos){//シャント抵抗を対象に追加する
                        element[element.size() - 1].shunt_det = 2; //shunt_detの値を2(シャント抵抗を対象に追加)に変更
                        element.push_back({0, line_num, name, node1, node2, triple_digits(value), unit,0, 0, range.LMIN, range.LMAX, 0, 0, 0, 0, ""});
                    }
                    //flg_shunt_B = 0;
                }
                flg_shunt_B = 0;
            }

            if(line.find("*MIN") == 0){
                double MIN;
                line.insert(line.find("=") ,"  ");
                line.insert(line.find("=") + 1 ,"  ");
                liness << line;
                liness >> ignore >> ignore >>  MIN;
                (element.back()).MIN = MIN;
            }
            else if(line.find("*MAX") == 0){
                double MAX;
                line.insert(line.find("=") ,"  ");
                line.insert(line.find("=") + 1 ,"  ");
                liness << line;
                liness >> ignore >> ignore >>  MAX;
                (element.back()).MAX = MAX;
            }
            else if(line.find("*FIX") == 0){ 
                (element.back()).FIX = 1;
            }
            else if(line.find("*SYN") == 0){
                double SYN;
                line.insert(line.find("=") ,"  ");
                line.insert(line.find("=") + 1 ,"  ");
                liness << line;
                liness >> ignore >> ignore >>  SYN;
                (element.back()).SYN = SYN;
            }


            range = find_range(range, line);
            flg_shunt_B = 0;


            switch (judge_element(line)) {
                case 0:
                    liness << line;    // 文字列lineをstringstream liness に格納
                    liness >> name >> node1 >> node2 >> main; //linessをname(素子名), node1, node2, main(それ以降)に分割

                    i1 = main.find("h");    //単位の切り離し(H)
                    i2 = main.find("H");
                    if ( i1 != string::npos || i2 != string::npos){
                        i = max({(int)i1,(int)i2});
                        main.insert(i," ");     
                    }
                    
                    sub = sub_unit(main);
                    unit = sub + "H";
                    mainss << main;
                    mainss >> value;

                    transform(name.begin(), name.end(), name.begin(), ::toupper); 
                    element.push_back({0, line_num, name, node1, node2, triple_digits(value), unit,0, 0, range.LMIN, range.LMAX, 0, 0, 0, 0, ""});
                    break;
                case 1:
                    liness << line;
                    liness >> name >> node1 >> node2 >> value;
                    transform(name.begin(), name.end(), name.begin(), ::toupper); 
                    element.push_back({1, line_num, name, node1, node2, triple_digits(value), "",0, 0, range.KMIN, range.KMAX, 0, 0, 0, 0, ""});
                    break;
                case 2:     // B
                    i1 = line.rfind("=");
                    if (i1 != string::npos){
                        line.insert(i1 + 1, " ");
                    }
                    liness << line;                    //jjmod    area=
                    liness >> name >> node1 >> node2 >> jjmod >> ignore >> value;
                    transform(name.begin(), name.end(), name.begin(), ::toupper); 
                    element.push_back({2, line_num, name, node1, node2, triple_digits(value), "",0, 0, range.BMIN, range.BMAX, 0, 0, -1, 0, jjmod});
                    flg_shunt_B = 1;
                    break;
                case 3:
                    i1 = line.rfind("=");
                    if (i1 != string::npos){
                        line.insert(i1 + 1, " ");
                    }
                    liness << line;
                    liness >> name >> node1 >> node2 >> jjmod >> ignore >> value;
                    transform(name.begin(), name.end(), name.begin(), ::toupper); 
                    element.push_back({3, line_num, name, node1, node2, triple_digits(value), sub,0, 0, range.BIMIN, range.BIMAX, 0, 0, 0, 0, jjmod});
                    break;
                case 4:  //C
                    liness << line;
                    liness >> name >> node1 >> node2 >> main;

                    i1 = main.find("f");
                    i2 = main.find("F");
                    if ( i1 != string::npos || i2 != string::npos){
                        i = max({(int)i1,(int)i2});
                        main.insert(i," ");     
                    }

                    sub = sub_unit(main);
                    unit = sub + "F";
                    mainss << main;
                    mainss >> value;

                    transform(name.begin(), name.end(), name.begin(), ::toupper); 
                    element.push_back({5, line_num, name, node1, node2, triple_digits(value), unit,0, 0, range.CMIN, range.CMAX, 0, 0, 0, 0, ""});
                    break;
                case 5:
                    i1 = line.rfind("ohm");    //Rの場合は、"ohm" がsub_unitのミリ(m)に引っかかるので、先に"ohm"を排除
                    i2 = line.rfind("OHM");
                    if ( i1 != string::npos || i2 != string::npos){
                        i = max({(int)i1,(int)i2});
                        line.insert(i," ");     
                    }

                    liness << line;
                    liness >> name >> node1 >> node2 >> main;

                    sub = sub_unit(main);
                    unit = sub + "ohm";
                    mainss << main;
                    mainss >> value;

                    transform(name.begin(), name.end(), name.begin(), ::toupper); 
                    element.push_back({5, line_num, name, node1, node2, triple_digits(value), unit,0, 0, range.RMIN, range.RMAX, 0, 0, 0, 0, ""});
                    break;
                case 6:
                    liness << line;
                    liness >> name >> node1 >> node2 >> ignore >> ignore  >> ignore >> main;
                    i1 = main.rfind("v");
                    i2 = main.rfind("V");
                    if ( i1 != string::npos || i2 != string::npos){
                        i = max({(int)i1,(int)i2});
                        main.insert(i," ");     
                    }

                    sub = sub_unit(main);
                    unit = sub + "V";
                    mainss << main;
                    mainss >> value;

                    liness << line;
                    liness >> name >> node1 >> node2 >> ignore >> ignore  >> ignore >> value;
                    transform(name.begin(), name.end(), name.begin(), ::toupper); 
                    element.push_back({6, line_num, name, node1, node2, triple_digits(value), unit,0, 0, range.IMIN, range.IMAX, 0, 0, 0, 0, ""});
                    break;
                case 7:
                    liness << line;
                    liness >> name >> node1 >> node2 >> ignore >> ignore  >> ignore >> main;

                    i1 = main.rfind("a");
                    i2 = main.rfind("A");
                    if ( i1 != string::npos || i2 != string::npos){
                        i = max({(int)i1,(int)i2});
                        main.insert(i," ");     
                    }

                    sub = sub_unit(main);
                    unit = sub + "A";
                    mainss << main;
                    mainss >> value;

                    transform(name.begin(), name.end(), name.begin(), ::toupper); 
                    element.push_back({7, line_num, name, node1, node2, triple_digits(value), unit,0, 0, range.VMIN, range.VMAX, 0, 0, 0, 0, ""});
                    break;
                    
                default:
                    break;

            }

        if(line.find(".FILE") == 0 || line.find(".file") == 0 || line.find(".File") == 0){// .FILE の行があったら、file_check == 1とする
            file_check = 1;
        }
        line_num++;
    }
    
    if(element.size() == 0){
        cout << " Error : No target element." << endl;
        return;
    }
    if(file_check == -1){ 
        cerr << "Please insert \".FILE\" line into your circuit file.";
        return;
    }

    file.close();



    vector<ele_unit> element_order;  //整列されたelement配列

    for(int ide_num = 0; ide_num <= 7; ide_num++){   //elementを整列
        for (size_t i = 0; i < element.size(); i++){
            if(element[i].ide_num == ide_num){
                element_order.push_back(element[i]);
            }
        }
    }

    element = element_order;




    /*
    vector<ele_unit> element_order;  //整列されたelement配列
    vector<int> line_num_order;
    for(int ide_num = 0; ide_num <= 7; ide_num++){   //elementを整列
        for (int i = 0; i < element.size(); i++){
            if(element[i].ide_num == ide_num){
                element_order.emplace_back(element[i]);
                line_num_order.emplace_back(element[i].line_num);
            }
        }
    }
    for(int i = 0; i < element.size(); i++){
            cout << element[i].element << " " << element[i].ide_num << " " << cou->line_num_arr[i] << endl ;
    }    
    element = element_order;
    cou->line_num_arr = line_num_order;

    for(int i = 0; i < element.size(); i++){
            cout << element[i].element  << "   " <<element[i].value << endl ;
        }    */
    }




