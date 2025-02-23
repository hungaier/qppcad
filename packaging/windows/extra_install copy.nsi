; custom.nsi

!define MYPREFIX "PythonPath"
!define MYUNINSTALL "Uninstall"

; 定义全局函数
Function .onInit
    StrCpy $0 ""
    Push $0
    Call, findPythonPaths
    Pop $0
    StrCpy $R0 $0
    StrCpy $0 ""
    StrCpy $R1 "Select Python Path"
    StrCpy $R2 $R0
    Call, .createList
    Pop $0
    StrCpy $0 "$R1"
    ListAddString $R2 $0
FunctionEnd

Function findPythonPaths
    ; 搜索安装目录
    StrCpy $R0 "${INSTDIR}"
    Call, searchDirectoryForPython

    ; 搜索用户目录
    StrCpy $R0 "${USERPROFILE}"
    Call, searchDirectoryForPython
FunctionEnd

Function searchDirectoryForPython
    ; 搜索指定目录及其子目录中的 python.exe
    StrCpy $0 ""
    StrCpy $R1 "$R0"
    StrCpy $R2 "*.exe"
    StrCpy $R3 "python.exe"
    StrCpy $R4 $0
    Call, searchFilesRecursively
    Pop $0
    StrCpy $0 "$0 $R4"
FunctionEnd

Function searchFilesRecursively
    ; 递归搜索
    FindFirst $R1 $R2 $R3 $0
    ${If} $0 != ""
        ; 检查文件版本
        StrCpy $1 ""
        ExecWait '"$0" --version' $1
        StrCpy $2 ""
        StrCpy $3 ""
        StrCpy $4 ""
        StrCpy $5 ""

        ; 提取主版本号和次版本号
        StringCpy $2 $1 8
        StringCpy $3 $1 7
        StringCpy $4 $1 6
        StringCpy $5 $1 5
        StrCmp $2 "Python 3.12"
        ${If} eq
            StrCmp $R4 $0
            ${If} ne
                StrCpy $R4 "$R4 $0"
            ${EndIf}
            StrCpy $0 $R4
            FindNext $R1 $R2 $R3 $0
            ${While} $0 != ""
                ExecWait '"$0" --version' $1
                StringCpy $2 $1 8
                StringCpy $3 $1 7
                StringCpy $4 $1 6
                StringCpy $5 $1 5
                StrCmp $2 "Python 3.12"
                ${If} eq
                    StrCmp $R4 $0
                    ${If} ne
                        StrCpy $R4 "$R4 $0"
                    ${EndIf}
                    StrCpy $0 $R4
                    FindNext $R1 $R2 $R3 $0
                ${Else}
                    FindNext $R1 $R2 $R3 $0
                ${EndIf}
            ${EndWhile}
        ${Else}
            FindNext $R1 $R2 $R3 $0
        ${EndIf}
    ${EndIf}
FunctionEnd

Function .createList
    ; 创建列表
    StrCpy $0 ""
    StrCpy $R1 "Python Paths"
    StrCpy $R2 $0
    Call, .createListBoxItems
    Pop $0
    StrCpy $0 "$R1"
    ListAddString $R2 $0
FunctionEnd

Function .createListBoxItems
    ; 添加列表项
    StrCpy $0 ""
    StrCpy $R1 $0
    Call, searchFilesRecursively
    Pop $0
    StrCpy $0 "$R1"
    ListAddString $R2 $0
FunctionEnd

Section "SelectPython" SEC_SELECTPYTHON
    ; 在欢迎页面后插入自定义页面
    !insertmacro MUI_PAGE_WELCOME
    !insertmacro MUI_PAGE_COMPONENTS
    Call .onInit
    MessageBox MB_OKCANCEL|MB_SETFOREGROUND "Select Python Path" "Choose the Python path you want to use:"
    ${If} $0 == "CA"
        Abort
    ${EndIf}

    ; 检查是否找到了Python路径
    ${If} $R0 == ""
        MessageBox MB_OKCANCEL|MB_SETFOREGROUND "No Python 3.12 Found" "No Python 3.12 was found on your system. Please select a valid Python path."
        ${If} $0 == "CA"
            Abort
        ${EndIf}
        StrCpy $0 ""
        StrCpy $R1 "Select Python Path"
        FileOpen $0 FILE SELECTFILE
        FileSelectAnyFile $0 $R1
        ${If} $0 == ""
            MessageBox MB_OKCANCEL|MB_SETFOREGROUND "No Selection" "No Python path selected."
            Abort
        ${EndIf}
        FileClose $0
        StrCpy $R2 $0
    ${Else}
        StrCpy $R2 $R0
    ${EndIf}

    ; 显示选择框
    Call, .createList
    !insertmacro MUI_PAGE_LISTBOX "Python Path" $R1 $R2
    !insertmacro MUI_PAGE_DIRECTORY
    !insertmacro MUI_PAGE_INSTFILES

    ; 用户选择后写入配置文件
    ${If} $0 == ""
        MessageBox MB_OKCANCEL|MB_SETFOREGROUND "Error" "No Python path selected."
        Abort
    ${EndIf}
    FileWrite "${INSTDIR}\app.config" "PYTHONPATH=$0"
SectionEnd