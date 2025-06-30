#include <iostream>
#include "/home/ljz/XXXVMP/XXX/VMPInterface.h"

int afunction [[protect]] (int a,int b=3){
    if(a<10){
        a++;
    }else{
        a--;
    }
    for(int i=0;i<10;i++){
        a+=i;
    }
    return a;
}

int a=5;
int main(){
    int b=2;
    int d = a+b;
    std::cout<<"d 的地址为："<<&d<<" d的值为："<<d<<" a的值为："<<a<<std::endl;
    a=10;
    int c = afunction(a,b);
    std::cout<<"c 的地址为："<<&c<<" c的值为："<<c<<" a的值为："<<a<<std::endl;
    return 0;
}