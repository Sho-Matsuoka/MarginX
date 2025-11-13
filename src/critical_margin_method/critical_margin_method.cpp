#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <thread>
#include <vector>
#include <chrono>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <iomanip> 
#include <cmath>
#include <random>
#include <regex>
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include "function.hpp"

using namespace std;

void critical_margin_method(vector<ele_unit> &element,  vector<vector<judge>> &jud, vector<string> &data_cir, vector<string> &arg_arr){
    stringstream commandname, delete_cir, delete_out, out_det, out_fig, out_csv;
    commandname << JOSIM_COMMAND << " OPTIMIZE" << getpid() << ".cir > /dev/null"; 
    delete_cir << "rm -f OPTIMIZE" << getpid() << ".cir";
    delete_out << "rm -f CIRCUIT" << getpid() << ".CSV";

    Margin(element,  jud, data_cir, arg_arr, 2);

    vector<ele_unit> copy = element;
    //ためしにすべての素子を中央値に変更する
/*
    for(int i = 0; i < copy.size(); i++){
        if(copy[i].FIX != 1){
            //cout << element[i].element << " : " << element[i].margin_L << ", " << element[i].margin_H << endl;
            //element[i].value = ( (element[i].margin_H + element[i].margin_L) / 200 + 1 ) * element[i].value;
            if(copy[i].value > 0){
                copy[i].value +=  ((copy[i].margin_H + copy[i].margin_L) / 2) / 100 * copy[i].value;
            }
            else{
                copy[i].value -=  ((copy[i].margin_H + copy[i].margin_L) / 2) / 100 * copy[i].value;
            }
            if(copy[i].value < copy[i].MIN){
                copy[i].value = copy[i].MIN;
            }

        }
    }
    //全て中央値に変換した回路が正常動作するか確認
    make_cir_opt(copy, data_cir);
    if(system((commandname.str()).c_str()) == -1){
        cout << "error:1" << endl;
    }
    //正常動作したら
    else if(judge_operation( jud, 0) == 1){ 
        Margin(copy,  jud, data_cir, arg_arr, 1); //全て中央値に変更した際のクリティカルマージンをチェック
        //全て中央値に変更した際のクリティカルマージンがもとのものより大きかったら、elementの値をcopyに格納していた中央値に変更し、正常動作しなければelementの値はそのまま（スルー）
        if(min({-copy[find_critical(copy)].margin_L, copy[find_critical(copy)].margin_H}) > min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H})){
            element = copy;
        for(int i = 0; i < element.size(); i++){
            if(element[i].value < element[i].MIN){  //新たなパラメータが最小値を下回っていたらパラメータを最小値に置き換える
                    element[i].value = element[i].MIN;
            }
            if(element[i].value > element[i].MAX){  //新たなパラメータが最大値を上回っていたらパラメータを最大値に置き換える
                    element[i].value = element[i].MAX;
            }
        }            cout << endl << " Changed All Elements" << endl;
            Margin(element,  jud, data_cir, arg_arr, 0);
        }
    }
    system((delete_cir.str()).c_str());
    system((delete_out.str()).c_str());

*/
    int cri_num_pre = -1;
    int cri_num;
    for(int j = 0; j < CRITICAL_NUM; j++){
        cri_num = find_critical_CMM(element);
        if(cri_num_pre == cri_num){
            cout << " " << element[cri_num].element << " is the critical element again." << endl;
            break;
        }
        if(element[cri_num].FIX == 1){
            cout << "chan't change the parameter of " <<  element[cri_num].element << " (FIX)" << endl;
            break;
        }
        cri_num_pre = cri_num;
        cout << endl << " ~ Critical Margin Method  "<<  GetOrdinalNumber(static_cast<int>(j + 1)) << " trial" << " ~" << endl;
        cout << " Critical Element : " << element[cri_num].element << endl;
        if(element[cri_num].value > 0){
            element[cri_num].value +=  ((element[cri_num].margin_H + element[cri_num].margin_L) / 2) / 100 * element[cri_num].value;
        }
        else{
            element[cri_num].value -=  ((element[cri_num].margin_H + element[cri_num].margin_L) / 2) / 100 * element[cri_num].value;
        }
        if(element[cri_num].value < element[cri_num].MIN){
            element[cri_num].value = element[cri_num].MIN;
        }
        if(element[cri_num].value > element[cri_num].MAX){
            element[cri_num].value = element[cri_num].MAX;
        }        
        synchro_opt(element, cri_num);
        Margin_low(element, jud, data_cir, arg_arr, 2);

    }
    cout << " Critical Margin Method is over!" << endl; 
    Margin_low(element, jud, data_cir, arg_arr, 0);

}


