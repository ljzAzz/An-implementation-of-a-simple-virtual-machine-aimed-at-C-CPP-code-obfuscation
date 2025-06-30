#pragma once 
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <stdlib.h>
#include <cstring>
#include <memory>


class InterFace
{
    private:
    static inline std::vector<size_t> m_offset={};
    static inline void* m_native_args=nullptr;
    static inline bool m_is_void=false;
    static inline void* ret=nullptr;
    public:
    template<typename T,typename... Args>
    InterFace(bool isVoid, T&& t,Args&&... args)
    {
        m_is_void=isVoid;
        m_offset.reserve(sizeof...(args)+1);

        size_t offset = 0;
        m_offset.push_back(offset);
        if(m_is_void){
            m_native_args=malloc(sizeof(T));
            std::memcpy(m_native_args,&t,sizeof(T));
            offset += sizeof(T);
            m_offset.push_back(offset);

        }else{
            ret=t;
        }


        for(auto arg : {args...}){
            offset += sizeof(arg);
            m_offset.push_back(offset);  
            m_native_args = realloc(m_native_args,offset); 
            std::memcpy((char*)m_native_args+m_offset.back(),reinterpret_cast<char*>(&arg),sizeof(arg));
        }
    }
    void setRetType(bool ret)
    {
        m_is_void = ret;
    }
    static void* getRet()
    {
        return ret;

    }
    static void* getNativeArgs()
    {
        return m_native_args;
    }
    static std::vector<size_t>& getOffset()
    {
        return m_offset;
    }
    static bool getIsVoid()
    {
        return m_is_void;
    }
    ~InterFace()
    {

    }
};


extern int RUN();


//懒得写了，这里可以自动化生成接口函数
template<typename T,typename... Args>
int InitVMPnative(bool isVoid,T&& t,Args&&... args){
    std::cout << "InitVMPnative" << std::endl;
    std::unique_ptr<InterFace> iface = std::make_unique<InterFace>(isVoid,std::forward<T>(t),std::forward<Args>(args)...);
    std::cout<<"InitVMPnative end" << std::endl;

    RUN();
    if(!isVoid){
        return *static_cast<int*>(t);
    }else{
        return 0;
    }
}
