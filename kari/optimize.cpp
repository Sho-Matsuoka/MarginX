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

void optimize(vector<ele_unit> &element, vector<string> &data_cir, vector<vector<judge>> &jud, vector<string> &arg_arr){


    int mode = 0;
    double CM_power = 1;
    double BM_power = 1;
    int count = 0;
    time_t start, now;
    string sharp = "";
    gauss *gau;
    cout << " Select the Kind of Score" << endl;
    cout << " 1: Only Critical Margin " << endl;
    cout << " 2: Only Bias Margin" << endl;
    cout << " 3: The Sum of Critical Margin and Bias Margin" << endl;
    cout << " 4: The Sum of Critical Margin and Bias Margin * 2 " << endl;
    cout << " 5: Others (input yourself)" << endl;
    cout << "" << endl;
    cout << " Selected Score : ";
    cin >> mode;

    switch(mode){
        case 1:
            CM_power = 1;
            BM_power = 0;
            break;
        case 2:
            CM_power = 0;
            BM_power = 1;
            break;
        case 3:
            CM_power = 1;
            BM_power = 1;
            break;
        case 4:
            CM_power = 1;
            BM_power = 2;
            break;
        case 5:
            cout << " Select Critical Margin Power" << endl;
            cout << " Critical Margin Power : ";
            cin >> CM_power;
            cout << " Select Bias Margin Power" << endl;
            cout << " Bias Margin Power : ";
            cin >> BM_power;
            break;
        default:
            cout << " Please Select a Correct Number" << endl;
            return;
            break;
    }

    start = time(NULL);
    opt_num *opt;
    int shmid;
    double cri_bias_sum = 0;
    int CMM_num = 1;
    vector<ele_unit> element_ini = element;
    cout << endl;
    if ((shmid = shmget(IPC_PRIVATE, sizeof(opt_num), 0666)) == -1) {
        cout << "can't make shared memory" << endl;
        exit(EXIT_FAILURE);
    }
    /*結果を格納する top に共有メモリをアタッチ*/
    opt = (opt_num *)shmat(shmid, NULL, 0);

    for (int m = 0; m < MONTE_CARLO; m++){
        if( m % (MONTE_CARLO / 5 ) == 0){
            critical_margin_method(element,  jud, data_cir, arg_arr);
            opt->suc_max = 0;
            cri_bias_sum = CM_power * min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) + BM_power * min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
            if( cri_bias_sum > opt->cri_bias_best ){
                for(int j = 0; j < element.size(); j++){
                    opt->best_value[j] = element[j].value;
                }
                opt->best_value[element.size()] = CMM_num;
                opt->cri_bias_best = cri_bias_sum;
            }
            //cout << "cri_bias_sum : " << cri_bias_sum << endl; 
            CMM_num++;
        }
        double global = rand_global(element);
        vector<int> pid;
        //マルチプロセス開始
        for (int i = 0; i < MULTI_NUM; ++i) { //MULTI_NUM = 1回のモンテカルロで生成するマルチプロセス数
                pid.emplace_back(fork());
                if(pid[i] == 0){       
                    opt_num  *shmaddr;
                    if ((shmaddr = (opt_num*)shmat(shmid, NULL, 0)) == (void *)-1) {
                        printf("childL can't load memory\n");
                        exit(EXIT_FAILURE);
                    }
                    opt_ele(element,data_cir,jud,shmaddr,m ,i, global);
                    //opt_ele2(element,data_cir,cou,jud,shmaddr,gau);
                    if (shmdt(shmaddr) == -1) {
                        exit(EXIT_FAILURE);
                    }
                    exit(0);
                }
                else if(pid[i] < 0){
                    cout << "can't make child process" << endl;
                    exit(0);
                }


        }
        //子プロセスの終了を待機
        for (int i = 0; i < MULTI_NUM; i++) {
            wait(NULL);
        }

        now = time(NULL);
        double progress = static_cast<double>(m + 1) / static_cast<double>(MONTE_CARLO) * 100;
        if(static_cast<int>(progress) - count >= 2){
            sharp += "O";
            count = static_cast<int>(progress);
        }
        //cout << "\x1B[1K";
        cout << " Optimizing...   "   << " [" << setw(50) << left <<  sharp << "]" <<setw(6) << right << progress << " % " <<  right << setw(5) <<static_cast<int>(static_cast<double>(now - start) / static_cast<double>(m+ 1) * (MONTE_CARLO - (m + 1))) << " seconds remaining"
                                                    << "   ( success : " << opt->success << " )" << endl;
        cout << "\x1B[1B";    //カーソルを１行下に移動させる
        cout << "\x1B[1A";    //カーソルを１行上に移動させる

        if( opt->success != 0 ){

            //if(opt->success >= opt->suc_max){  //成功数が過去最大以上だったら値を置き換える
                //opt->suc_max = opt->success;
                //cout << " changed value" << endl;
            for(int i = 0; i < element.size(); i++){
                element[i].value = opt->sum_value[i] / opt->success;
                
                if(element[i].value < element[i].MIN){
                        element[i].value = element[i].MIN;
                }
                if(element[i].value > element[i].MAX){
                        element[i].value = element[i].MAX;
                }
                if(element[i].FIX == 1){
                    element[i].value = element_ini[i].value;
                }
            }
            
        }
        opt->success = 0;
        for(int i = 0; i < element.size(); i++){
            opt->sum_value[i] = 0;
        }
        
    }
    critical_margin_method(element,  jud, data_cir, arg_arr);
    cri_bias_sum = CM_power * min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) + BM_power * min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
    //cout << "cri_bias_sum : " << cri_bias_sum << endl; 
    if( cri_bias_sum > opt->cri_bias_best ){
        for(int j = 0; j < element.size(); j++){
            opt->best_value[j] = element[j].value;
        }
        opt->best_value[element.size()] = CMM_num; // 配列best_valueの一番最後の要素の次の要素にスコアが最も良いCMMの回数を格納
        opt->cri_bias_best = cri_bias_sum;
    }
    cout <<  " This is the " << GetOrdinalNumber(static_cast<int>(opt->best_value[element.size()])) << " value." << endl;
    for(int i = 0; i < element.size(); i++){
        element[i].value = opt->best_value[i];
    }

    Margin(element,  jud, data_cir, arg_arr, 0);

    make_cir_last(element, data_cir, arg_arr);

    //確保していた共有メモリを解放
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        cout << "can't delete shared memory" << endl;
        exit(EXIT_FAILURE);
    }


}


void opt_ele(vector<ele_unit> &element, vector<string> &data_cir, vector<vector<judge>> &jud, opt_num *opt, int m, int i, double global){

    std::random_device rnd;
    std::mt19937 mt(rnd()); 

    int power = 4;

    //double sigma = ( element[find_critical(element)].margin_H - element[find_critical(element)].margin_L ) / 2 / 1000 * 5;
    double sigma = min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) / 1000 * power;   // 3 ? 5 ? I dont know which number is correct.
    //cout << "sigma = " << sigma << endl;
    //sigma = 0.17;
    std::normal_distribution<> norm(1, sigma);    // (平均, 分散)
    std::normal_distribution<> normG(1, sigma);    // (平均, 分散)
    vector<ele_unit> copy = element;   //現在の各素子の情報elementをcopyにコピー
    stringstream commandname, delete_cir, delete_out;
    commandname << "josim OPTIMIZE" << getpid() << ".cir > /dev/null"; 
    delete_cir << "rm -f OPTIMIZE" << getpid() << ".cir";
    delete_out << "rm -f CIRCUIT" << getpid() << ".CSV";

/*
    double global_L = abs(normG(mt));
    double global_K = abs(normG(mt));    
    double global_B = abs(normG(mt));    
    double global_BI = abs(normG(mt));    
    double global_PI = abs(normG(mt));    
    double global_R = abs(normG(mt));    
    double global_V = abs(normG(mt));   
    double global_I = abs(normG(mt));    
*/

    double local;

    double judge = 0;


    for(int j = 0; j < copy.size(); j++){
        if(copy[j].FIX != 1){
            switch(copy[j].ide_num){
                case 0 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 1 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 2 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 3 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 4 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 5 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 6 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 7 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                default :  break;
            }
            
            if(copy[j].value < copy[j].MIN){  //新たなパラメータが最小値を下回っていたらパラメータを最小値に置き換える
                    copy[j].value = copy[j].MIN;
            }
            if(copy[j].value > copy[j].MAX){  //新たなパラメータが最大値を上回っていたらパラメータを最大値に置き換える
                    copy[j].value = copy[j].MAX;
            }
            if(copy[j].FIX == 1){
                copy[j].value = element[j].value;
            }

            synchro_opt(copy, j);
        }
        //cout << copy[j].element << " : " << copy[j].value << endl;
    }
    make_cir_opt(copy,data_cir);
    if(system((commandname.str()).c_str()) == -1){
        cout << "error:1" << endl;
    }    judge = judge_operation( jud, 0);
    if(judge == 1){ //正常動作したら
        opt->success += 1;
        for(int i = 0; i < copy.size(); i++){
            opt->sum_value[i] += copy[i].value;
        }
    }
    else if(judge == 0){
        for(int i = 0; i < copy.size(); i++){
            opt->sum_value_f[i] += copy[i].value;
        }
    }
/*
    opt->param[m * 100 + i][0] = copy[0].value;
    opt->param[m * 100 + i][1] = copy[1].value;
    opt->param[m * 100 + i][2] = judge;
    */
    system((delete_cir.str()).c_str());
    system((delete_out.str()).c_str());




}


void critical_margin_method(vector<ele_unit> &element,  vector<vector<judge>> &jud, vector<string> &data_cir, vector<string> &arg_arr){
    stringstream commandname, delete_cir, delete_out, out_det, out_fig, out_csv;
    commandname << "josim OPTIMIZE" << getpid() << ".cir > /dev/null"; 
    delete_cir << "rm -f OPTIMIZE" << getpid() << ".cir";
    delete_out << "rm -f CIRCUIT" << getpid() << ".CSV";

    Margin(element,  jud, data_cir, arg_arr, 2);

    vector<ele_unit> copy = element;
    //ためしにすべての素子を中央値に変更する
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


    int cri_num_pre = -1;
    int cri_num;
    for(int j = 0; j < CRITICAL_NUM; j++){
        cri_num = find_critical(element);
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
        Margin(element,  jud, data_cir, arg_arr, 2);

    }
    cout << " Critical Margin Method is over!" << endl; 

}



void optimize_monte(vector<ele_unit> &element, vector<string> &data_cir, vector<vector<judge>> &jud, vector<string> &arg_arr){

    int mode = 0;
    double CM_power = 1;
    double BM_power = 1;
    int count = 0;
    int bias_margin = 0;
    double lambda;
    double global = 0;
    time_t start, now;
    string sharp = "";
    string yield = "yield.csv";
    string param = "param.csv";
    ofstream fp_yield(yield);
    ofstream fp_param(param);
    gauss *gau;

    cout << " Select the Kind of Score" << endl;
    cout << " 1: Only Critical Margin " << endl;
    cout << " 2: Only Bias Margin" << endl;
    cout << " 3: The Sum of Critical Margin and Bias Margin" << endl;
    cout << " 4: The Sum of Critical Margin and Bias Margin * 2 " << endl;
    cout << " 5: Others ( input yourself )" << endl << endl;
    cout << "  Selected Score : ";
    cin >> mode;

    switch(mode){
        case 1:
            CM_power = 1;
            BM_power = 0;
            break;
        case 2:
            CM_power = 0;
            BM_power = 1;
            break;
        case 3:
            CM_power = 1;
            BM_power = 1;
            break;
        case 4:
            CM_power = 1;
            BM_power = 2;
            break;
        case 5:
            cout << " Select Critical Margin Power" << endl;
            cout << " Critical Margin Power : ";
            cin >> CM_power;
            cout << " Select Bias Margin Power" << endl;
            cout << " Bias Margin Power : ";
            cin >> BM_power;
            break;
        default:
            cout << " Please Select a Correct Number" << endl;
            return;
            break;
    }

    start = time(NULL);

    
    opt_num *opt;
    int shmid;
    double cri_bias_sum = 0;
    int Margin_num = 1;
    vector<ele_unit> element_ini = element;
    cout <<  endl;
    if ((shmid = shmget(IPC_PRIVATE, sizeof(opt_num), 0666)) == -1) {
        cout << "can't make shared memory" << endl;
        exit(EXIT_FAILURE);
    }
    //結果を格納する opt に共有メモリをアタッチ
    opt = (opt_num *)shmat(shmid, NULL, 0);
    //Margin(element,  jud, data_cir, cou, arg_arr, 0);
    //Margin(element,  jud, data_cir, cou, arg_arr, 0);
    for (int m = 0; m < MONTE_CARLO; m++){
        //途中経過
        if( m % (MONTE_CARLO / 10 ) == 0){
            opt->suc_max = 0; 
            cout << " This is the progress midway   ( the " << GetOrdinalNumber(static_cast<int>(Margin_num)) 
                 << " check )                                                                                             " << endl;
            Margin(element,  jud, data_cir, arg_arr, 2);
            make_cir_last(element, data_cir, arg_arr);            
            //cri_bias_sum = min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
            
            bias_margin =  min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
            cri_bias_sum = CM_power * min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) + BM_power * bias_margin;
            if( cri_bias_sum > opt->cri_bias_best ){
                for(int j = 0; j < element.size(); j++){
                    opt->best_value[j] = element[j].value;
                }
                opt->best_value[element.size()] = Margin_num;
                opt->cri_bias_best = cri_bias_sum;
            }
            //cout << "cri_bias_sum : " << cri_bias_sum << endl; 
            Margin_num++;
        }
        global = rand_global(element);
        vector<int> pid;
        //マルチプロセス開始
        for (int i = 0; i < MULTI_NUM; ++i) { // MULTI_NUM = 1回のモンテカルロで生成するマルチプロセス数
                pid.emplace_back(fork());
                if(pid[i] == 0){       
                    opt_num  *shmaddr;
                    if ((shmaddr = (opt_num*)shmat(shmid, NULL, 0)) == (void *)-1) {
                        cout << "childL can't load memory" << endl;
                        exit(EXIT_FAILURE);
                    }
                    opt_ele(element,data_cir,jud,shmaddr, m , i, global);
                    if (shmdt(shmaddr) == -1) {
                        exit(EXIT_FAILURE);
                    }
                    exit(0);
                }
                else if(pid[i] < 0){
                    cout << "can't make child process" << endl;
                    exit(0);
                }


        }
        //子プロセスの終了を待機
        for (int i = 0; i < MULTI_NUM; i++) {
            wait(NULL);
        }

        now = time(NULL);
        double progress = static_cast<double>(m + 1) / static_cast<double>(MONTE_CARLO) * 100;
        if(static_cast<int>(progress) - count >= 2){
            sharp += "O";
            count = static_cast<int>(progress);
        }
        //cout << "\x1B[1K";
        cout << " Optimizing...   "   << " [" << setw(50) << left <<  sharp << "]" <<setw(6) << right << progress << " % " <<  right << setw(5) <<static_cast<int>(static_cast<double>(now - start) / static_cast<double>(m+ 1) * (MONTE_CARLO - (m + 1))) << " seconds remaining"
                                                    << "   ( success : " << opt->success << " )" << endl;
        cout << "\x1B[1B";    //カーソルを１行下に移動させる
        cout << "\x1B[1A";    //カーソルを１行上に移動させる
        //cout << "\r";

        
            fp_yield << m % 50<< ", " << opt->success << endl;



        if( opt->success != 0 ){    // 開発者確認用
            //cout << "success = " << left << setw(5) << opt->success;
            //for(int s = 0; s < opt->success; s++){
                //cout << "#";
            //}
            //cout << endl;

            //if(opt->success >= opt->suc_max){  //成功数が過去最大以上だったら値を置き換える <- 毎回置き換えた方がいい結果が出ていたので不要(?)
                //opt->suc_max = opt->success;
            //cout << " changed value" << endl;
        lambda = (MULTI_NUM - opt->success) / MULTI_NUM;
        for(int i = 0; i < element.size(); i++){
            element[i].value = opt->sum_value[i] / opt->success;   // 新たなパラメータに置き換える
            //element[i].value = opt->sum_value[i] / opt->success + lambda * (element[i].value - opt->sum_value_f[i] / MULTI_NUM);   // 新たなパラメータに置き換える
            if(element[i].value < element[i].MIN){  //新たなパラメータが最小値を下回っていたらパラメータを最小値に置き換える
                    element[i].value = element[i].MIN;
            }
            if(element[i].value > element[i].MAX){  //新たなパラメータが最大値を上回っていたらパラメータを最大値に置き換える
                    element[i].value = element[i].MAX;
            }
            if(element[i].FIX == 1){
                element[i].value = element_ini[i].value;
            }
        }
        

        }
        /*
        if( opt->success >= opt->suc_max){
            opt->suc_max = opt->success;
            for(int i = 0; i < element.size(); i++){
                opt->best_value[i] =element[i].value;
            }
        }*/
        opt->success = 0;
        for(int i = 0; i < element.size(); i++){
            opt->sum_value[i] = 0;
            opt->sum_value_f[i] = 0;
        }


    }

    Margin(element,  jud, data_cir, arg_arr, 2);
    //cri_bias_sum = min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
    cri_bias_sum = CM_power * min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) + BM_power * min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
    // cri_bias_sum = その回路のマージンの評価 (クリティカルマージン + 2 * バイアスマージン)
    //cout << "cri_bias_sum : " << cri_bias_sum << endl; 
    if( cri_bias_sum > opt->cri_bias_best ){   // マージンの評価値 cri_bias_sum が最大だったらそれを現時点の最良の回路として置き換える
        for(int j = 0; j < element.size(); j++){
            opt->best_value[j] = element[j].value;    // best_value配列に現時点の最良の回路のパラメータを格納
        }
        opt->best_value[element.size()] = Margin_num;  //best_value配列の最後の要素の次の要素に何番目の回路かを格納
        opt->cri_bias_best = cri_bias_sum;   //cri_bias_best を更新
    }

    cout <<  " This is the " << GetOrdinalNumber(static_cast<int>(opt->best_value[element.size()])) << " value." << endl;
    for(int i = 0; i < element.size(); i++){
        element[i].value = opt->best_value[i];  //最終的な最良の回路のパラメータをelement配列に格納
    }

    Margin(element,  jud, data_cir, arg_arr, 0);

    make_cir_last(element, data_cir, arg_arr);

    fp_param << element[0].element << ", " << element[1].element << ", " << "judge" << endl;

/*
    for(int col = 0; col < 50000; col++){
        for(int row = 0; row < 3; row++){
            if(row != 2){
                fp_param << (opt->param[col][row] - element_ini[row].value) / element_ini[row].value * 100;
                fp_param << ", ";
            }
            else{
                fp_param << opt->param[col][row];
            }
        }
        fp_param << endl;
    }*/

    /*確保していた共有メモリを解放*/
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        cout << "can't delete shared memory" << endl;
        exit(EXIT_FAILURE);
    }
    fp_yield.close();        

}


void optimize_monte_ul(vector<ele_unit> &element, vector<string> &data_cir, vector<vector<judge>> &jud, vector<string> &arg_arr){
    int mode = 0;
    int CM_choice, BM_choice;
    double CM_th = 0;
    double BM_th = 0;
    double CM_power = 1;
    double BM_power = 1;
    time_t start, now;
    string sharp = "";
    gauss *gau;

    cout << " Do you Set the MIN of the Critical Margin ?" << endl;
    cout << " 1 : Yes" << endl << " 2 : No" << endl << " Selected : ";
    cin >> CM_choice;
    if(CM_choice == 1){
        cout << endl << " Please input the threshold of Critical Margin : " ;
        cin >> CM_th;
    }
    cout << " Do you Set the MIN of the Bias Margin ?" << endl;
    cout << " 1 : Yes" << endl << " 2 : No" << endl << " Selected : ";
    cin >> BM_choice;
    if(BM_choice == 1){
        cout << endl << " Please input the threshold of Bias Margin : " ;
        cin >> BM_th;
    }
    if(CM_choice != 1 && BM_choice != 1){
        cout << endl <<  " Please set the threshold of Margin!" << endl;
        return;
    }

    cout << " Select the Kind of Score" << endl;
    cout << " 1: Only Critical Margin " << endl;
    cout << " 2: Only Bias Margin" << endl;
    cout << " 3: The Sum of Critical Margin and Bias Margin" << endl;
    cout << " 4: The Sum of Critical Margin and Bias Margin * 2 " << endl;
    cout << " 5: Others ( input yourself )" << endl << endl;
    cout << "  Selected Score : ";
    cin >> mode;

    switch(mode){
        case 1:
            CM_power = 1;
            BM_power = 0;
            break;
        case 2:
            CM_power = 0;
            BM_power = 1;
            break;
        case 3:
            CM_power = 1;
            BM_power = 1;
            break;
        case 4:
            CM_power = 1;
            BM_power = 2;
            break;
        case 5:
            cout << " Select Critical Margin Power" << endl;
            cout << " Critical Margin Power : ";
            cin >> CM_power;
            cout << " Select Bias Margin Power" << endl;
            cout << " Bias Margin Power : ";
            cin >> BM_power;
            break;
        default:
            cout << " Please Select a Correct Number" << endl;
            return;
            break;
    }

    start = time(NULL);
    
    opt_num *opt;
    int shmid;
    double cri_bias_sum = 0;
    int Margin_num = 1;
    int m = -1;
    vector<ele_unit> element_ini = element;
    cout <<  endl;
    if ((shmid = shmget(IPC_PRIVATE, sizeof(opt_num), 0666)) == -1) {
        cout << "can't make shared memory" << endl;
        exit(EXIT_FAILURE);
    }
    //結果を格納する top に共有メモリをアタッチ
    opt = (opt_num *)shmat(shmid, NULL, 0);
    //Margin(element,  jud, data_cir, cou, arg_arr, 0);
    //Margin(element,  jud, data_cir, cou, arg_arr, 0);
    while(1){
        m++;
        //途中経過
        if( m % 100 == 0){
            opt->suc_max = 0; 
            cout << " This is the progress midway   ( the " << GetOrdinalNumber(static_cast<int>(m / 100 + 1)) 
                 << " check )                                                                                             " << endl;
            Margin(element,  jud, data_cir, arg_arr, 2);
            if( min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) > CM_th  &&  min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H}) > BM_th ){
                make_cir_last(element, data_cir, arg_arr);            
                cout << " Achieved the goal!!!" << endl;
                break;
            }         
            cri_bias_sum = CM_power * min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) + BM_power * min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
            if( cri_bias_sum > opt->cri_bias_best ){
                for(int j = 0; j < element.size(); j++){
                    opt->best_value[j] = element[j].value;
                }
                make_cir_last(element, data_cir, arg_arr);            
                opt->best_value[element.size()] = Margin_num;
                opt->cri_bias_best = cri_bias_sum;
            }
            //cout << "cri_bias_sum : " << cri_bias_sum << endl; 
            Margin_num++;
        }
        double global = rand_global(element);
        vector<int> pid;
        //マルチプロセス開始
        for (int i = 0; i < MULTI_NUM; ++i) { // MULTI_NUM = 1回のモンテカルロで生成するマルチプロセス数
                pid.emplace_back(fork());
                if(pid[i] == 0){       
                    opt_num  *shmaddr;
                    if ((shmaddr = (opt_num*)shmat(shmid, NULL, 0)) == (void *)-1) {
                        cout << "childL can't load memory" << endl;
                        exit(EXIT_FAILURE);
                    }
                    opt_ele(element,data_cir,jud,shmaddr,m ,i, global);
                    if (shmdt(shmaddr) == -1) {
                        exit(EXIT_FAILURE);
                    }
                    exit(0);
                }
                else if(pid[i] < 0){
                    cout << "can't make child process" << endl;
                    exit(0);
                }


        }
        //子プロセスの終了を待機
        for (int i = 0; i < MULTI_NUM; i++) {
            wait(NULL);
        }

        now = time(NULL);

        //cout << "\x1B[1K";
        cout << " Optimizing...   "   << "Monte Carlo Count : " << m + 1 <<  right << setw(10) <<static_cast<int>(static_cast<double>(now - start)) << " seconds passed"
                                                    << "   ( success : " << opt->success << " )" << endl;
        cout << "\x1B[1B";    //カーソルを１行下に移動させる
        cout << "\x1B[1A";    //カーソルを１行上に移動させる
        //cout << "\r";

        if( opt->success != 0 ){    // 開発者確認用
            //cout << "success = " << left << setw(5) << opt->success;
            //for(int s = 0; s < opt->success; s++){
                //cout << "#";
            //}
            //cout << endl;

            //if(opt->success >= opt->suc_max){  //成功数が過去最大以上だったら値を置き換える <- 毎回置き換えた方がいい結果が出ていたので不要(?)
                //opt->suc_max = opt->success;
            //cout << " changed value" << endl;

        for(int i = 0; i < element.size(); i++){
            element[i].value = opt->sum_value[i] / opt->success;   // 新たなパラメータに置き換える
            
            if(element[i].value < element[i].MIN){  //新たなパラメータが最小値を下回っていたらパラメータを最小値に置き換える
                    element[i].value = element[i].MIN;
            }
            if(element[i].value > element[i].MAX){  //新たなパラメータが最大値を上回っていたらパラメータを最大値に置き換える
                    element[i].value = element[i].MAX;
            }
            if(element[i].FIX == 1){
                element[i].value = element_ini[i].value;
            }
        }
        

        }
        /*
        if( opt->success >= opt->suc_max){
            opt->suc_max = opt->success;
            for(int i = 0; i < element.size(); i++){
                opt->best_value[i] =element[i].value;
            }
        }*/
        opt->success = 0;
        for(int i = 0; i < element.size(); i++){
            opt->sum_value[i] = 0;
        }


    }


    /*確保していた共有メモリを解放*/
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        cout << "can't delete shared memory" << endl;
        exit(EXIT_FAILURE);
    }


}

string GetOrdinalNumber(int num){
    string res = to_string(num);
    if(num == 11 || num == 12){
        res += "th";
    }
    else{
        switch (num % 10){
        case 1:
            res += "st";
            break;
        case 2:
            res += "nd";
            break;

        case 3:
            res += "rd";
            break;

        default:
            res += "th";
            break;
        }
    }
    return res;
} 

/*
void opt_ele2(vector<ele_unit> &element, vector<string> &data_cir, ele_cou *cou, vector<vector<judge>> &jud, opt_num *opt, gauss *gau){

    std::random_device rnd;
    std::mt19937 mt(rnd()); 
    std::normal_distribution<> norm(1, 0.05);    // (平均, 分散)
    vector<ele_unit> copy = element;   //現在の各素子の情報elementをcopyにコピー
    stringstream commandname, delete_cir, delete_out;
            commandname << "josim OPTIMIZE" << getpid() << ".cir > /dev/null"; 
    delete_cir << "rm -f OPTIMIZE" << getpid() << ".cir";
    delete_out << "rm -f CIRCUIT" << getpid() << ".CSV";

    double local_L = abs(norm(mt));
    double local_K = abs(norm(mt));    
    double local_B = abs(norm(mt));    
    double local_BI = abs(norm(mt));    
    double local_PI = abs(norm(mt));    
    double local_R = abs(norm(mt));    
    double local_V = abs(norm(mt));   
    double local_I = abs(norm(mt));    
    //double local;



    for(int j = 0; j < copy.size(); j++){
        switch(copy[j].ide_num){
            case 0 :  
                copy[j].value = copy[j].value * gau->global_L * local_L;
                break;
            case 1 :  
                copy[j].value = copy[j].value * gau->global_K * local_K;
                break;
            case 2 :  
                copy[j].value = copy[j].value * gau->global_B * local_B;
                break;
            case 3 :  
                copy[j].value = copy[j].value * gau->global_BI * local_BI;
                break;
            case 4 :  
                copy[j].value = copy[j].value * gau->global_PI * local_PI;
                break;
            case 5 :  
                copy[j].value = copy[j].value * gau->global_R * local_R;
                break;
            case 6 :  
                copy[j].value = copy[j].value * gau->global_V * local_V;
                break;
            case 7 :  
                copy[j].value = copy[j].value * gau->global_I * local_I;
                break;
            default :  break;
        }
        //cout << copy[j].element << " : " << copy[j].value << endl;
    }
    make_cir_opt(copy,data_cir,cou);
    if(system((commandname.str()).c_str()) == -1){
        cout << "error:1" << endl;
    }
    else if(judge_operation( jud, 0) == 1){ //正常動作したら
        opt->success += 1;
        for(int i = 0; i < copy.size(); i++){
            opt->sum_value[i] += copy[i].value;
        }
    }

    system((delete_cir.str()).c_str());
    system((delete_out.str()).c_str());



}
*/

double rand_globa(vector<ele_unit> &element){
    std::random_device rnd;
    std::mt19937 mt(rnd()); 

    int power = 4;
    //double sigma = ( element[find_critical(element)].margin_H - element[find_critical(element)].margin_L ) / 2 / 1000 * 5;
    double sigma = min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) / 1000 * power;   // 3 ? 5 ? I dont know which number is correct.

    //cout << "sigma = " << sigma << endl;
    //sigma = 0.17;
    std::normal_distribution<> norm(1, sigma);    // (平均, 分散)
    vector<ele_unit> copy = element;   //現在の各素子の情報elementをcopyにコピー

    double global  = abs(norm(mt));

/*
    gau->global_L  = abs(norm(mt));
    gau->global_K  = abs(norm(mt));
    gau->global_B  = abs(norm(mt));
    gau->global_BI = abs(norm(mt));
    gau->global_PI = abs(norm(mt)); 
    gau->global_R  = abs(norm(mt));
    gau->global_V  = abs(norm(mt));
    gau->global_I  = abs(norm(mt));
*/
    return global;
}




void optimize_yield(vector<ele_unit> &element, vector<string> &data_cir, vector<vector<judge>> &jud, vector<string> &arg_arr){

    int mode = 0;
    double CM_power = 1;
    double BM_power = 1;
    int count = 0;
    int bias_margin = 0;
    double yield_ave = 0;
    double local_nd = 0.1;
    double lambda;
    int not_upd = 0;
    int bunbo = 100;
    time_t start, now;
    string sharp = "";
    string yield = "yield.csv";
    string param = "param.csv";
    ofstream fp_yield(yield);
    ofstream fp_param(param);
    gauss *gau;
    vector<int> yield_log;
    yield_log.push_back(50);
    //cout << yield_log[0] << endl;

    cout << " Select the Kind of Score" << endl;
    cout << " 1: Only Critical Margin " << endl;
    cout << " 2: Only Bias Margin" << endl;
    cout << " 3: The Sum of Critical Margin and Bias Margin" << endl;
    cout << " 4: The Sum of Critical Margin and Bias Margin * 2 " << endl;
    cout << " 5: Others ( input yourself )" << endl << endl;
    cout << "  Selected Score : ";
    cin >> mode;

    switch(mode){
        case 1:
            CM_power = 1;
            BM_power = 0;
            break;
        case 2:
            CM_power = 0;
            BM_power = 1;
            break;
        case 3:
            CM_power = 1;
            BM_power = 1;
            break;
        case 4:
            CM_power = 1;
            BM_power = 2;
            break;
        case 5:
            cout << " Select Critical Margin Power" << endl;
            cout << " Critical Margin Power : ";
            cin >> CM_power;
            cout << " Select Bias Margin Power" << endl;
            cout << " Bias Margin Power : ";
            cin >> BM_power;
            break;
        default:
            cout << " Please Select a Correct Number" << endl;
            return;
            break;
    }

    start = time(NULL);

    
    opt_num *opt;
    int shmid;
    double cri_bias_sum = 0;
    int Margin_num = 1;
    vector<ele_unit> element_ini = element;
    //opt->suc_max = 0;
    cout <<  endl;
    if ((shmid = shmget(IPC_PRIVATE, sizeof(opt_num), 0666)) == -1) {
        cout << "can't make shared memory" << endl;
        exit(EXIT_FAILURE);
    }
    //結果を格納する opt に共有メモリをアタッチ
    opt = (opt_num *)shmat(shmid, NULL, 0);
    for (int m = 0; m < MONTE_CARLO; m++){
        //最初
        if( m % 100 == 0){
            opt->suc_max = 0; 
            cout << " This is the progress midway   (the " << GetOrdinalNumber(static_cast<int>(Margin_num)) 
                 << " check)                                                                                             " << endl;
            Margin(element,  jud, data_cir, arg_arr, 2);
            make_cir_last(element, data_cir, arg_arr);            
            //cri_bias_sum = min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
            
            bias_margin =  min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
            cri_bias_sum = CM_power * min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) + BM_power * bias_margin;
            if( cri_bias_sum > opt->cri_bias_best ){
                for(int j = 0; j < element.size(); j++){
                    opt->best_value[j] = element[j].value;
                }
                opt->best_value[element.size()] = Margin_num;
                opt->cri_bias_best = cri_bias_sum;
            }
            //cout << "cri_bias_sum : " << cri_bias_sum << endl; 
            Margin_num++;
        }   // end if( m == 0)


/*
        //開始：歩留まりから標準偏差を計算 
        int yield_ite = 0;
        int yield_arr_num = yield_log.size() - 1;
        //cout << yield_arr_num << endl;
        int sum = 0;
        while(yield_ite < 5){
            if(yield_arr_num - yield_ite < 0){
                break;
            }
            //cout << yield_log[yield_arr_num - yield_ite] << endl;
            sum += yield_log[yield_arr_num - yield_ite];
            yield_ite++;
        }
        yield_ave = sum / 5;
        local_nd = 0.1 * ( 1 + (yield_ave - 50) / 50);
        //終了：歩留まりから標準偏差を計算 

*/


        //local_nd = 0.09;      ////////////////////////////////////////////////////////////

        if(not_upd >= 10){
            if(local_nd > 0.01){
                local_nd -= 0.01;
                opt->suc_max = 0;
            }
            else{
                goto END;
            }
            not_upd = 0;
        }


        double global = rand_global_yield(local_nd);  //グローバルな乱数計算

        //マルチプロセス開始
        vector<int> pid;        
        for (int i = 0; i < MULTI_NUM; i++) { // MULTI_NUM = 1回のモンテカルロで生成するマルチプロセス数
                pid.emplace_back(fork());
                if(pid[i] == 0){       
                    opt_num  *shmaddr;
                    if ((shmaddr = (opt_num*)shmat(shmid, NULL, 0)) == (void *)-1) {
                        cout << "childL can't load memory" << endl;
                        exit(EXIT_FAILURE);
                    }
                    opt_ele_yield(element,data_cir,jud,shmaddr, m , i, global, local_nd);
                    if (shmdt(shmaddr) == -1) {
                        exit(EXIT_FAILURE);
                    }
                    exit(0);
                }
                else if(pid[i] < 0){
                    cout << "can't make child process" << endl;
                    exit(0);
                }


        }
        //子プロセスの終了を待機
        for (int i = 0; i < MULTI_NUM; i++) {
            wait(NULL);
        }

        now = time(NULL);
        double progress = static_cast<double>(m + 1) / static_cast<double>(MONTE_CARLO) * 100;
        if(static_cast<int>(progress) - count >= 2){
            sharp += "O";
            count = static_cast<int>(progress);
        }
        //cout << "\x1B[1K";
        cout << " Optimizing...   "   << " [" << setw(50) << left <<  sharp << "]" <<setw(6) << right << progress << " % " <<  right << setw(5) <<static_cast<int>(static_cast<double>(now - start) / static_cast<double>(m+ 1) * (MONTE_CARLO - (m + 1))) << " seconds remaining"
                                                    << "   ( success : " << opt->success << " )" << ", ND = " << local_nd << ", not_upd = " << not_upd << endl;
        cout << "\x1B[1B";    //カーソルを１行下に移動させる
        cout << "\x1B[1A";    //カーソルを１行上に移動させる
        //cout << "\r";

        
        fp_yield << m + 1 << ", " << opt->success << ", " << element[1].value << endl;
        yield_log.push_back(opt->success);

        not_upd++;   //歩留まりが向上しなかったら、opt_num を+1する
        bunbo = MULTI_NUM - opt->success;
        if(opt->success == 100){
            bunbo = 1;
        }

        if( opt->success != 0 ){    // 開発者確認用
            lambda = (MULTI_NUM - opt->success) / MULTI_NUM;
            for(int i = 0; i < element.size(); i++){
                //element[i].value = opt->sum_value[i] / opt->success + lambda * (element[i].value - opt->sum_value_f[i] / bunbo);   // 新たなパラメータに置き換える
                
                if(element[i].value < element[i].MIN){  //新たなパラメータが最小値を下回っていたらパラメータを最小値に置き換える
                        element[i].value = element[i].MIN;
                }
                if(element[i].value > element[i].MAX){  //新たなパラメータが最大値を上回っていたらパラメータを最大値に置き換える
                        element[i].value = element[i].MAX;
                }
                if(element[i].FIX == 1){
                    element[i].value = element_ini[i].value;
                }
            }

            if( opt->success >= opt->suc_max){
                not_upd = 0;
                opt->suc_max = opt->success;

                for(int i = 0; i < element.size(); i++){
                    element[i].value = opt->sum_value[i] / opt->success;
                    //element[i].value = opt->sum_value[i] / opt->success + lambda * (element[i].value - opt->sum_value_f[i] / bunbo);   // 新たなパラメータに置き換える
                    //opt->best_value[i] =element[i].value;
                }
            }
        }
        opt->success = 0;
        for(int i = 0; i < element.size(); i++){
            opt->sum_value[i] = 0;
            opt->sum_value_f[i] = 0;
        }


    }
    END:
    Margin(element,  jud, data_cir, arg_arr, 2);
    //cri_bias_sum = min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
    cri_bias_sum = CM_power * min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) + BM_power * min({-element[find_critical_bias(element)].margin_L, element[find_critical_bias(element)].margin_H});
    // cri_bias_sum = その回路のマージンの評価 (クリティカルマージン + 2 * バイアスマージン)
    //cout << "cri_bias_sum : " << cri_bias_sum << endl; 
    if( cri_bias_sum > opt->cri_bias_best ){   // マージンの評価値 cri_bias_sum が最大だったらそれを現時点の最良の回路として置き換える
        for(int j = 0; j < element.size(); j++){
            opt->best_value[j] = element[j].value;    // best_value配列に現時点の最良の回路のパラメータを格納
        }
        opt->best_value[element.size()] = Margin_num;  //best_value配列の最後の要素の次の要素に何番目の回路かを格納
        opt->cri_bias_best = cri_bias_sum;   //cri_bias_best を更新
    }

    cout <<  " This is the " << GetOrdinalNumber(static_cast<int>(opt->best_value[element.size()])) << " value." << endl;
    for(int i = 0; i < element.size(); i++){
        element[i].value = opt->best_value[i];  //最終的な最良の回路のパラメータをelement配列に格納
    }

    critical_margin_method(element,  jud, data_cir, arg_arr);

    Margin(element,  jud, data_cir, arg_arr, 0);

    make_cir_last(element, data_cir, arg_arr);

    //fp_param << element[0].element << ", " << element[1].element << ", " << "judge" << endl;

/*
    for(int col = 0; col < 50000; col++){
        for(int row = 0; row < 3; row++){
            if(row != 2){
                fp_param << (opt->param[col][row] - element_ini[row].value) / element_ini[row].value * 100;
                fp_param << ", ";
            }
            else{
                fp_param << opt->param[col][row];
            }
        }
        fp_param << endl;
    }
*/
    /*確保していた共有メモリを解放*/
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        cout << "can't delete shared memory" << endl;
        exit(EXIT_FAILURE);
    }
    fp_yield.close();        

}



void opt_ele_yield(vector<ele_unit> &element, vector<string> &data_cir, vector<vector<judge>> &jud, opt_num *opt, int m, int i, double global, double local_nd){

    std::random_device rnd;
    std::mt19937 mt(rnd()); 

    int power = 10;

    //double sigma = ( element[find_critical(element)].margin_H - element[find_critical(element)].margin_L ) / 2 / 1000 * 5;
    //double sigma = min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) / 1000 * power;   // 3 ? 5 ? I dont know which number is correct.
    //cout << "sigma = " << sigma << endl;
    //sigma = 0.17;
    std::normal_distribution<> norm(1, local_nd);    // (平均, 標準偏差)
    vector<ele_unit> copy = element;   //現在の各素子の情報elementをcopyにコピー
    stringstream commandname, delete_cir, delete_out;
    commandname << "josim OPTIMIZE" << getpid() << ".cir > /dev/null"; 
    delete_cir << "rm -f OPTIMIZE" << getpid() << ".cir";
    delete_out << "rm -f CIRCUIT" << getpid() << ".CSV";

    double local;

    double judge = 0;


    for(int j = 0; j < copy.size(); j++){
        if(copy[j].FIX != 1){
            switch(copy[j].ide_num){
                case 0 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 1 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 2 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 3 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 4 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 5 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 6 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                case 7 :  
                    local = abs(norm(mt));
                    copy[j].value = copy[j].value * local * global;
                    break;
                default :  break;
            }
            
            if(copy[j].value < copy[j].MIN){  //新たなパラメータが最小値を下回っていたらパラメータを最小値に置き換える
                    copy[j].value = copy[j].MIN;
            }
            if(copy[j].value > copy[j].MAX){  //新たなパラメータが最大値を上回っていたらパラメータを最大値に置き換える
                    copy[j].value = copy[j].MAX;
            }
            if(copy[j].FIX == 1){
                copy[j].value = element[j].value;
            }
            synchro_opt(copy, j);
        }
        //cout << copy[j].element << " : " << copy[j].value << endl;
    }
    make_cir_opt(copy,data_cir);
    if(system((commandname.str()).c_str()) == -1){
        cout << "error:1" << endl;
    }

    judge = judge_operation( jud, 0);
    if(judge == 1){ //正常動作したら
        opt->success += 1;
        for(int i = 0; i < copy.size(); i++){
            opt->sum_value[i] += copy[i].value;
        }
    }
    else if(judge == 0){
        for(int i = 0; i < copy.size(); i++){
            opt->sum_value_f[i] += copy[i].value;
        }
    }
/*
    opt->param[m * 100 + i][0] = copy[0].value;
    opt->param[m * 100 + i][1] = copy[1].value;
    opt->param[m * 100 + i][2] = judge;*/
    system((delete_cir.str()).c_str());
    system((delete_out.str()).c_str());
}

double rand_global_yield(double nd){
    std::random_device rnd;
    std::mt19937 mt(rnd()); 

    std::normal_distribution<> norm(1, nd);    // (平均, 分散)

    return  abs(norm(mt));
}

double rand_global(vector<ele_unit> &element){
    int power = 4;

    //double sigma = ( element[find_critical(element)].margin_H - element[find_critical(element)].margin_L ) / 2 / 1000 * 5;
    double sigma = min({-element[find_critical(element)].margin_L, element[find_critical(element)].margin_H}) / 1000 * power;   // 3 ? 5 ? I dont know which number is correct.
    std::random_device rnd;
    std::mt19937 mt(rnd()); 

    std::normal_distribution<> norm(1, sigma);    // (平均, 分散)

    return  abs(norm(mt));
}