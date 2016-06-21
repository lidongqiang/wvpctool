#include "tinyxml.h"
#include "xmlib.h"
#define ROOT_NAME "SNWTool"
int test()
{
    return 0;
}

void readSchoolXml() {
    //using namespace std;
    const char * xmlFile = "conf/school.xml";
    TiXmlDocument doc;                              
    if (doc.LoadFile(xmlFile)) {
        doc.Print();
    } else {
        //cout << "can not parse xml conf/school.xml" << endl;
        return;
    }
    TiXmlElement* rootElement = doc.RootElement();  //School元素  
    TiXmlElement* classElement = rootElement->FirstChildElement();  // Class元素
    TiXmlElement* studentElement = classElement->FirstChildElement();  //Students  
    for (; studentElement != NULL; studentElement = studentElement->NextSiblingElement() ) {
        TiXmlAttribute* attributeOfStudent = studentElement->FirstAttribute();  //获得student的name属性  
        for (;attributeOfStudent != NULL; attributeOfStudent = attributeOfStudent->Next() ) {
            //cout << attributeOfStudent->Name() << " : " << attributeOfStudent->Value() << std::endl;       
        }                                 

        TiXmlElement* studentContactElement = studentElement->FirstChildElement();//获得student的第一个联系方式 
        for (; studentContactElement != NULL; studentContactElement = studentContactElement->NextSiblingElement() ) {
            //string contactType = studentContactElement->Value();
            //string contactValue = studentContactElement->GetText();
            //cout << contactType  << " : " << contactValue << std::endl;           
        }   
     
    } 
}
int openconf(char * name,unsigned int flags)
{
    TiXmlDocument   *pdoc;
    bool            loadOkay;
    if(0 == name){
        return 0;
    }
    pdoc = new TiXmlDocument(name);
    if(!pdoc){
        return 0;
    }
    loadOkay = pdoc->LoadFile();
    return (int)pdoc;
    /*
    loadOkay = pdoc->LoadFile();
    if( loadOkay ) {
        return (int)pdoc;
    } else {
        delete pdoc;
        pdoc = 0;
        return 0;
    }
    */
}
bool closeconf(int hconf,unsigned int flags)
{
    TiXmlDocument   *pdoc;
    pdoc            = (TiXmlDocument*)hconf;
    if(!pdoc){
        return false;
    }
    if(flags) {
        pdoc->SaveFile();
    }
    delete pdoc;
    return true;
}
//
bool writevalue(int hconf,const char *field,char *value )
{
    TiXmlNode       *node               = 0;
    TiXmlElement    *rootElement        = 0;
    TiXmlElement    *fieldElement       = 0;
    TiXmlElement    *newldElement       = 0;
    TiXmlDocument   *pdoc               = 0;
    TiXmlText       *pTextValue         = 0;
    TiXmlNode       *pTextNode          = 0;
    if(hconf){
        pdoc = (TiXmlDocument*)hconf;
    } else {
        return false;
    }
    node = pdoc->RootElement();
    if(!node){                              //here we need create a new node
        node = new TiXmlElement(ROOT_NAME);
        pdoc->LinkEndChild(node);    
                                            //root element no exist,the create a new element
    }
    if(strcmp(ROOT_NAME,node->Value())){
        //delete and create a new one
        return false;
    }
    rootElement     = node->ToElement(); //we come to <SNWTool>   </SNWTool> element
    fieldElement    = rootElement->FirstChildElement(field);
    if(fieldElement){
        pTextNode       = fieldElement->FirstChild();
        if(pTextNode) {
            pTextNode->SetValue(value);
        } else {
            pTextValue  = new TiXmlText(value);
            fieldElement->LinkEndChild(pTextValue);
        }
    } else {
        pTextValue      = new TiXmlText(value);
        fieldElement    = new TiXmlElement(field);
        fieldElement->LinkEndChild(pTextValue);
        rootElement->LinkEndChild( fieldElement );
    }
    return true;
}
bool readvalue(int hconf,const char *field,char *value,int *cnt)
{
    TiXmlNode       *node               = 0;
    TiXmlElement    *rootElement        = 0;
    //TiXmlElement    *settingElement     = 0;
    TiXmlElement    *fieldElement       = 0;
    TiXmlElement    *newldElement       = 0;
    TiXmlDocument   *pdoc               = 0;
    TiXmlText       *pTextValue         = 0;
    TiXmlNode       *pTextNode          = 0;
    const char      *tvalue             = 0;
    //every node no exist we should return false
    if(hconf){
        pdoc = (TiXmlDocument*)hconf;
    } else {
        return false;
    }
    node = pdoc->RootElement();
    if(!node){
        return false;
    }
    if(strcmp(ROOT_NAME,node->Value())){
        //delete and create a new one
        return false;
    }
    rootElement     = node->ToElement(); //we enter <SNWTool>   </SNWTool> element
    fieldElement  = rootElement->FirstChildElement(field);
    if(fieldElement){
        pTextNode   = fieldElement->FirstChild();
        if(pTextNode) {
            tvalue  = pTextNode->Value();
            if(strlen(tvalue) < *cnt - 1){
                strcpy_s(value,*cnt,tvalue);
                *cnt = strlen(tvalue);
                return true;
            }
        }
    } 
    return false;
}
bool writeconf(int hconf,char *option,char *field,char *value )
{
    TiXmlNode       *node               = 0;
    TiXmlElement    *rootElement        = 0;
    TiXmlElement    *settingElement     = 0;
    TiXmlElement    *fieldElement       = 0;
    TiXmlElement    *newldElement       = 0;
    TiXmlDocument   *pdoc               = 0;
    TiXmlText       *pTextValue         = 0;
    TiXmlNode       *pTextNode          = 0;
    if(hconf){
        pdoc = (TiXmlDocument*)hconf;
    } else {
        return false;
    }
    node = pdoc->RootElement();
    if(!node){                              //here we need create a new node
        node = new TiXmlElement(ROOT_NAME);
        pdoc->LinkEndChild(node);    
                                            //root element no exist,the create a new element
    }
    if(strcmp(ROOT_NAME,node->Value())){
        //delete and create a new one
        return false;
    }
    rootElement     = node->ToElement(); //we come to <SNWTool>   </SNWTool> element
    settingElement  = rootElement->FirstChildElement(option);
    if( settingElement) {
        fieldElement    = settingElement->FirstChildElement(field);
        if(fieldElement){
            pTextNode       = fieldElement->FirstChild();
            if(pTextNode) {
                pTextNode->SetValue(value);
            } else {
                pTextValue  = new TiXmlText(value);
                fieldElement->LinkEndChild(pTextValue);
            }
        } else {
            pTextValue      = new TiXmlText(value);
            fieldElement    = new TiXmlElement(field);
            fieldElement->LinkEndChild(pTextValue);
            settingElement->LinkEndChild( fieldElement );
        }
    } else {
        pTextValue      = new TiXmlText(value);
        fieldElement    = new TiXmlElement(field);
        fieldElement->LinkEndChild(pTextValue);
        settingElement  = new TiXmlElement(option);
        settingElement->LinkEndChild( fieldElement );
        rootElement->LinkEndChild( settingElement );
    } 
    return true;
}
bool writeconfattr(int hconf,char *option,char *field,char *name,char *value )
{
    TiXmlNode       *node               = 0;
    TiXmlElement    *rootElement        = 0;
    TiXmlElement    *settingElement     = 0;
    TiXmlElement    *fieldElement       = 0;
    TiXmlElement    *newldElement       = 0;
    TiXmlDocument   *pdoc               = 0;
    if(hconf){
        pdoc = (TiXmlDocument*)hconf;
    } else {
        return false;
    }
    node = pdoc->RootElement();
    if(!node){
        node = new TiXmlElement(ROOT_NAME);
        pdoc->LinkEndChild(node);    
        //root element no exist,the create a new element
    }
    if(strcmp(ROOT_NAME,node->Value())){
        //delete and create a new one
        return false;
    }
    rootElement     = node->ToElement(); //we come to <SNWTool>   </SNWTool> element
    settingElement  = rootElement->FirstChildElement(option);
    if( settingElement) {
        fieldElement    = settingElement->FirstChildElement(field);
        if(fieldElement){
            fieldElement->SetAttribute(name,value);
        } else {
            fieldElement    = new TiXmlElement(field);
            fieldElement->SetAttribute(name,value);
            settingElement->LinkEndChild( fieldElement );
        }
    } else {
        fieldElement    = new TiXmlElement(field);
        fieldElement->SetAttribute(name,value);
        settingElement  = new TiXmlElement(option);
        settingElement->LinkEndChild( fieldElement );
        rootElement->LinkEndChild( settingElement );
    } 
    return true;
}
/*******************************************************************************
*name       :bool readconf(int hconf,char *option,char *field,char *value )
*synopsis   :
*
*******************************************************************************/
bool readconf(int hconf,char *option,char *field,char *value,int *cnt)
{
    TiXmlNode       *node               = 0;
    TiXmlElement    *rootElement        = 0;
    TiXmlElement    *settingElement     = 0;
    TiXmlElement    *fieldElement       = 0;
    TiXmlElement    *newldElement       = 0;
    TiXmlDocument   *pdoc               = 0;
    TiXmlText       *pTextValue         = 0;
    TiXmlNode       *pTextNode          = 0;
    const char      *tvalue             = 0;
    //every node no exist we should return false
    if(hconf){
        pdoc = (TiXmlDocument*)hconf;
    } else {
        return false;
    }
    node = pdoc->RootElement();
    if(!node){
        return false;
    }
    if(strcmp(ROOT_NAME,node->Value())){
        //delete and create a new one
        return false;
    }
    rootElement     = node->ToElement(); //we enter <SNWTool>   </SNWTool> element
    settingElement  = rootElement->FirstChildElement(option);
    if( settingElement) {
        fieldElement    = settingElement->FirstChildElement(field);
        if(fieldElement){
            pTextNode   = fieldElement->FirstChild();
            if(pTextNode) {
                tvalue  = pTextNode->Value();
                if(strlen(tvalue) < *cnt - 1){
                    strcpy_s(value,*cnt,tvalue);
                    *cnt = strlen(tvalue);
                    return true;
                }
            }
        } 
    }
    return false;
}
bool writeconfstr(int hconf,char *field,char *name,char *value )
{
    return true;
}
bool writeconfint(char *name,int value )
{

    return true; 
}
bool writeconfchar(char *name,char value )
{

    return true; 
}
bool writeconfshort(char *name, short value )
{

    return true; 
}
void writeSchoolXml() {
    //using namespace std;
    const char * xmlFile = "conf/school-write.xml"; 
    TiXmlDocument doc;  
    TiXmlDeclaration * decl = new TiXmlDeclaration("1.0", "", "");  
    TiXmlElement * schoolElement = new TiXmlElement( "School" );  
    TiXmlElement * classElement = new TiXmlElement( "Class" );  
    classElement->SetAttribute("name", "C++");

    TiXmlElement * stu1Element = new TiXmlElement("Student");
    stu1Element->SetAttribute("name", "tinyxml");
    stu1Element->SetAttribute("number", "123");
    TiXmlElement * stu1EmailElement = new TiXmlElement("email");
    stu1EmailElement->LinkEndChild(new TiXmlText("tinyxml@163.com") );
    TiXmlElement * stu1AddressElement = new TiXmlElement("address");
    stu1AddressElement->LinkEndChild(new TiXmlText("中国"));
    stu1Element->LinkEndChild(stu1EmailElement);
    stu1Element->LinkEndChild(stu1AddressElement);

    TiXmlElement * stu2Element = new TiXmlElement("Student");
    stu2Element->SetAttribute("name", "jsoncpp");
    stu2Element->SetAttribute("number", "456");
    TiXmlElement * stu2EmailElement = new TiXmlElement("email");
    stu2EmailElement->LinkEndChild(new TiXmlText("jsoncpp@163.com"));
    TiXmlElement * stu2AddressElement = new TiXmlElement("address");
    stu2AddressElement->LinkEndChild(new TiXmlText("美国"));
    stu2Element->LinkEndChild(stu2EmailElement);
    stu2Element->LinkEndChild(stu2AddressElement);

    classElement->LinkEndChild(stu1Element);  
    classElement->LinkEndChild(stu2Element);  
    schoolElement->LinkEndChild(classElement);  
    
    doc.LinkEndChild(decl);  
    doc.LinkEndChild(schoolElement); 
    doc.SaveFile(xmlFile);  
}