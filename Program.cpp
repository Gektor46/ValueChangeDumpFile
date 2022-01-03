#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>
#include <time.h>

using namespace std;

// Структура типа:
// Переменная - строка
// Значение - строка
struct VAR
{
    VAR(const string& _vr, const string& _vl): var(_vr), value(_vl) {};
    string var, value;
};
 
int main(int argc, char **argv)
{
    setlocale(LC_ALL, "ru");
    int opt;

    // Если запускаем без аргументов, выводим справку
    if(argc == 1) 
    {
        cout << "Enter the file in this format:\n";
        cout << " Example(Var): $ ./Program.out -v FileName.vcd\n";
        cout << " or\n";
        cout << " Example(Time): $ ./Program.out -t FileName.vcd\n";
        cout << " or\n";
        cout << " Example(Section): $ ./Program.out -s 1000 FileName.vcd\n";
        cout << " or\n";
        cout << " Example(Incision): $ ./Program.out -i 10 100 1000 FileName.vcd\n";
        cout << " or\n";
        cout << " Example(Join): $ ./Program.out -j FileName1.vcd FileName2.vcd FileName3.vcd\n";
        return 0;
    }

    // Пока используется наш функционал и существуют аргументы для него
    while((opt = getopt(argc, argv, "v:t:s:i:j:")) != -1) 
    {
        ifstream VCDFile;
        fstream VCDNEW;
        string VCD;
        string temporaryVCDNEW;
        int wordcount = argc - 1; 
        VCDFile.open(argv[argc - 1], ifstream::app);
        // Если файла нет, он не открылся или он пуст, выдавать ошибку
        // При этом создается пустой файл, который пользователь может заполнить 
        if ((!VCDFile.is_open()) || (!VCDFile) || (VCDFile.peek() == EOF))
        {
            cout << "Error opening VCD file!\n";
            return 1;
        }
        VCDNEW.open("VCDNEW.vcd");
        if (!VCDNEW.is_open())
        {
            cout << "VCD file cannot be opened or created.\n";
            return 1;
        }
        
        //Кейсы с функциональностью
        switch(opt) 
        {
            //Кейс разбиения файлов на отдельно взятые по времени
            case 'i':
                cout << "\nSplitting VCD files into separately taken by time:\n\n";
            break; 
            //Кейс соединения файлов в один
            case 'j':
                
                bool resolution;
                time_t rawtime; 
                struct tm * timeinfo; 
                time ( &rawtime ); 
                timeinfo = localtime (&rawtime);
                //Шапка нового VCD
                VCDNEW << "$date\n\t" << asctime (timeinfo) << "$end\n";
                VCDNEW << "$version HDL\n\tVerifier version 1.0\n$end\n";
                VCDNEW << "$timescale 1ns $end\n";

                while (wordcount != 1)
                {    
                    while(getline(VCDFile, VCD))
                    {
                        if ((VCD.find("date") == 1) && (VCD.find("version") == 1) && (VCD.find("timescale") == 1))
                        {
                            resolution = false;
                            continue;
                        }
                        if ((VCD.find("end") == 1) || (resolution = false))
                        {
                            resolution = true;
                            continue;  
                        } 
                        getline(VCDNEW, temporaryVCDNEW);
                        
                        cout << temporaryVCDNEW;
                        if (temporaryVCDNEW.find(VCD) == string::npos)
                        {
                            VCDNEW << VCD << endl; 
                        }
                        temporaryVCDNEW.clear();
                    }
                    wordcount--;
                }
                cout << "\nAs a result of file join received <VCDNEW.vcd>\n\n";
            break;
            //Кейс вывода всех переменных, используемых в файле
            case 'v':
                cout << "\nVCD file variables:\n\n";
                while(getline(VCDFile, VCD))
                {
                    if (VCD.find("var") == 1)
                    {
                         string VarStr = VCD.substr(5, VCD.length() - 10);
                        cout << " " << VarStr << endl;
                    }
                }
            break;
            //Кейс вывода всех разрезов времени, используемых в файле 
            case 't':
                cout << "\nVCD file time sections:\n\n";
                while(getline(VCDFile, VCD))
                {
                    if (VCD.find("#") == 0)
                    {
                        string TimeStr = VCD.substr(1);
                        cout << " " << TimeStr << endl;
                    }
                }
            break;
            //Кейс вывода всех разрезов времени, используемых в файле
            case 's':
                vector<VAR> vars;
                vector<string> values;
                values.push_back("0");
                values.push_back("1");
                values.push_back("x");
                values.push_back("b");
                values.push_back("z");
                string space(" ");
                string star("*");
                size_t prev;
                size_t next;
                size_t delta = space.length();
                int var;
                int slice = atoi(argv[2]);
                cout << "\nThe value of the vcd file variables at the time: " << argv[2] << "\n\n";
                while(getline(VCDFile, VCD))
                {
                    //Цикл находит пременные и помещает их в вектор
                    if (VCD.find("$var") == 0)
                    {
                        string VarStr = VCD.substr(5, VCD.length() - 10);
                        var = 0;
                        prev = 0;
                        while((next = VarStr.find(space, prev)) != string::npos)
                        {
                            string tmp = VarStr.substr(prev, next-prev);
                            if (var == 2)
                            {
                                vars.reserve(1);
                                vars.push_back(VAR(tmp,""));
                            }
                            prev = next + delta;
                            var++;
                        }
                    }
                    // Цикл находит разрез времени и сравнивает его с числом пользователя
                    if (VCD.find("#") == 0)
                    {
                        int time = atoi(VCD.substr(1, VCD.length()).c_str());
                        if (time > slice)
                        {
                            break;
                        }
                        else if (time = slice)
                        {
                            continue;
                        } 
                    }
                    // Цикл помещает значения пременных в соответствующий вектор
                    if (find(values.begin(), values.end(), VCD.substr(0, 1)) != values.end())
                    {
                        string newvar;
                        string newvalue;
                        int AddressSpace = VCD.find(space, 0);
                        int AddressStar = VCD.find(star, 0);
                        if (AddressSpace != -1)
                        {
                            newvar = VCD.substr(AddressSpace + 1);
                            newvalue = VCD.substr(0, AddressSpace);
                        }
                        else 
                        {
                            if (AddressStar != -1)
                            {
                                newvar = VCD.substr(AddressStar);
                                newvalue = VCD.substr(0, 1);   
                            }
                            else
                            {
                                newvar = VCD.substr(1);
                                newvalue = VCD.substr(0, 1);   
                            }  
                        }
                        //Обновление значений
                        for (int elem = 0 ;vars.size() ; elem++)
                        { 
                            if (vars[elem].var == newvar)
                            {
                                vars[elem].value = newvalue;
                                break;
                            }
                        }
                    }
                }
                //Вывод актуальной информации
                for (int elem = 0; elem < vars.size(); elem++)
                {
                    cout << "Var: " << vars[elem].var << " Value: " << vars[elem].value << endl;
                }
            break;
        }    
        VCDFile.close();
    }
    return 0;
}