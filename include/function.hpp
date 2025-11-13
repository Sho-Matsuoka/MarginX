#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <vector>

#define JOSIM_COMMAND "josim"
#define JSIM_COMMAND  "jsim"   //JoSIMのコマンドを変更する位はここを書き換える
//file.cpp
#define SMALL_SUM 200           /* L_MAX + K_MAX + B_MAX + BI_MAX + PI_MAX + R_MAX + V_MAX + I_MAX */

//margin.cpp
#define MARGIN_UPPER 2          /* マージン測定の上限が100％ */
#define MARGIN_FIG "#"

//optimize.hpp
#define MULTI_NUM 100
#define MONTE_CARLO 500
#define CRITICAL_NUM 10


struct ele_unit{
    int ide_num;  // L = 0, K = 1, B = 2, BI = 3, C = 4, R = 5, V = 6, I = 7
    int line_num;
    std::string element;
    std::string node1;
    std::string node2;
    double value;  
    std::string unit;
    double margin_L;
    double margin_H;
    double MIN;
    double MAX;
    int FIX;  //1なら値をばらつかせない
    int SYN;
    int shunt_det= -1;   //Bc => 1,  IcRs => 0  unshunt => -1  push_to_element => 2  //shunt_det 
    double shunt_cal;   //shunt no keisanyou IcRs and Bc    shunt_cal
    std::string jjmod; //jjmod
};



struct range{
    double LMIN = 0;
    double LMAX = 1000;
    double KMIN = -1;
    double KMAX = 1;
    double BMIN = 0.1;
    double BMAX = 3;
    double BIMIN = 0;
    double BIMAX = 3;
    double CMIN = 0;
    double CMAX = 3;
    double RMIN = 0;
    double RMAX = 1000;
    double VMIN = -10;
    double VMAX = 10;
    double IMIN = -10;
    double IMAX = 10;
};

struct opt_num{
    int success = 0;
    int suc_max = 0;
    double best_value[SMALL_SUM] = {};
    double cri_bias_best = 0;
    //std::vector<double> sum_value;
    double sum_value[SMALL_SUM] = {};
    double sum_value_f[SMALL_SUM] = {};
    //double param[50000][3] = {};
};

struct judge{
    int btime;
    int etime;
    double phase;
    int anti;
};

struct board{
    double margin[SMALL_SUM][2];
};



//display
void display_title (std::vector<std::string> &arg_arr);
void display_help ();

//setup
void setup_python_path(std::vector<std::string> arg_arr);
void setup(std::vector<std::string> &arg_arr);
void setup_josim_command(std::vector<std::string> arg_arr);
void setup_jsim_command(std::vector<std::string> arg_arr);

//time
void end_time(time_t start, time_t end);

//menu
int menu();
int menu_jsim();

//read_file
int readJudgementFile(std::vector<std::vector<judge>> &jud, std::string &judgefilename, std::vector<std::string> arg_arr);
void make_data_cir(std::vector<std::string>& data_cir, std::string &filename, std::vector<ele_unit> &element);
int judge_element(std::string &line);
range find_range(range range, std::string line); //各パラメータの上限・下限をチェック
std::string sub_unit(std::string &main);
void convert_jsim(std::vector<std::string> &data_cir, std::string &filename);

//miscellaneous
void delete_file();
double triple_digits(double num);
int get_digit(double num);  //桁数を取得する関数 (□ × 10 ^ ○) の ○ の部分を取得
std::string GetOrdinalNumber(int num);
int search_command(std::vector<std::string> arg_arr, const std::string command);


//output
void detail_out(std::vector<ele_unit> &element);
void fig_out(std::vector<ele_unit> &element);
void file_out(std::string &filename, std::vector<ele_unit> &element);
int margin_py();

//synchro
void synchro(std::vector<ele_unit> &element, int ele_num, double syn_value);
void synchro_opt(std::vector<ele_unit> &element, int ele_num); 

//calc_margin
void Margin(std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud ,std::vector<std::string>& data_cir, std::vector<std::string>& arg_arr, int menu);
int margin_ele(int ele_num, std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud, board *top, std::vector<std::string> &data_cir);
void Margin_jsim(std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud ,std::vector<std::string>& data_cir, std::vector<std::string>& arg_arr, int menu);
int margin_ele_jsim(int ele_num, std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud, board *top, std::vector<std::string> &data_cir);


//calc_margin_low
void Margin_low(std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud ,std::vector<std::string>& data_cir, std::vector<std::string>& arg_arr, int menu);
int margin_ele_low(int ele_num, std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud, board *top, std::vector<std::string> &data_cir);
void Margin_low_jsim(std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud ,std::vector<std::string>& data_cir, std::vector<std::string>& arg_arr, int menu);
int margin_ele_low_jsim(int ele_num, std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud, board *top, std::vector<std::string> &data_cir);
void Margin_low_jsim_seq(std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud ,std::vector<std::string>& data_cir, std::vector<std::string>& arg_arr, int menu);

//calc_margin_syn
void Margin_syn(std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud ,std::vector<std::string>& data_cir, std::vector<std::string>& arg_arr, int menu);
int margin_ele_syn(int ele_num, std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud, board *top, std::vector<std::string> &data_cir);
void Margin_syn_jsim(std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud ,std::vector<std::string>& data_cir, std::vector<std::string>& arg_arr, int menu);
int margin_ele_syn_jsim(int ele_num, std::vector<ele_unit> &element,std::vector<std::vector<judge>> &jud, board *top, std::vector<std::string> &data_cir);

//critical_find
int find_critical(std::vector<ele_unit> &element);//クリティカルな素子を探索して、それが格納されているelement内の要素番号返す
int find_critical_bias(std::vector<ele_unit> &element);//バイアス電圧・電流内のクリティカルな素子を探索して、それが格納されているelement内の要素番号返す
int find_critical_CMM(std::vector<ele_unit> &element);//クリティカルな素子を探索して、それが格納されているelement内の要素番号返す

//critical_calc
double calc_critical(std::vector<ele_unit> &element);//クリティカルな素子を探索して、それが格納されているelement内の要素番号返す
double calc_critical_bias(std::vector<ele_unit> &element);//バイアス電圧・電流内のクリティカルな素子を探索して、それが格納されているelement内の要素番号返す
double calc_critical_upper(std::vector<ele_unit> &element);//クリティカルな素子を探索して、それが格納されているelement内の要素番号返す
double calc_critical_lower(std::vector<ele_unit> &element);//クリティカルな素子を探索して、それが格納されているelement内の要素番号返す

//score
double calc_score(std::vector<ele_unit> &element, std::vector<double> power); // mode配列の内容に応じてuserが選択した方式でスコアを計算
std::vector<double> select_score(); // mode の中身を決定 (userが入力)

//judgement
std::vector<std::vector<double>> readJOSIMData();
int judge_operation(std::vector<std::vector<judge>> &jud, int mode);
std::vector<std::vector<double>> readJSIMData();
int judge_operation_jsim(std::vector<std::vector<judge>> &jud, int mode);

//make_circuitfile
int make_cir(double new_value, int ele_num, std::vector<ele_unit> &element, std::vector<std::string>& data_cir);
int make_cir_opt(std::vector<ele_unit> &element, std::vector<std::string>& data_cir);
int make_cir_last(std::vector<ele_unit> &element, std::vector<std::string>& data_cir, std::vector<std::string> &arg_arr);


//critical_margin_method
void critical_margin_method(std::vector<ele_unit> &element, std::vector<std::vector<judge>> &jud ,std::vector<std::string>& data_cir, std::vector<std::string>& arg_arr);
void critical_margin_method_jsim(std::vector<ele_unit> &element, std::vector<std::vector<judge>> &jud ,std::vector<std::string>& data_cir, std::vector<std::string>& arg_arr);

/*
//optimize_monte
void optimize_monte(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, std::vector<std::string>& arg_arr);
void optimize_monte_ul(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, std::vector<std::string>& arg_arr);

//optimize_con
void optimize_yield(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, std::vector<std::string>& arg_arr);
void optimize_yield_up_down(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, std::vector<std::string>& arg_arr);
void optimize(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, std::vector<std::string>& arg_arr);
*/

//optimize_yield
void optimize_yield_up(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, std::vector<std::string>& arg_arr);
void optimize_yield_up_jsim(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, std::vector<std::string>& arg_arr);

//optimize_seq
void optimize_seq(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, std::vector<std::string>& arg_arr);
void optimize_seq_jsim(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, std::vector<std::string>& arg_arr);

//rand_local
void opt_ele(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud,opt_num *opt, int m, int i, double global);
void opt_ele_yield(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, opt_num *opt, double global, double local, std::vector<double> &global_rand);
void opt_ele_yield_jsim(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, opt_num *opt, double global, double local, std::vector<double> &global_rand);

//rand_global
double rand_global(std::vector<ele_unit> &element);
double rand_global_yield(double nd);

//search
double rand_global_search(double nd);
void search_ele(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, opt_num *opt, double global, double local, std::vector<double> &global_rand);
void search(std::vector<ele_unit> &element, std::vector<std::string> &data_cir, std::vector<std::vector<judge>> &jud, std::vector<std::string>& arg_arr);


#endif
