; custom.nsi

; 定义全局函数
;Function onInit
;    StrCpy $0 ""
;FunctionEnd


;Section "SelectPython"
    ; 在欢迎页面后插入自定义页面
    !insertmacro MUI_PAGE_WELCOME
    !insertmacro MUI_PAGE_COMPONENTS
    ;Call onInit
    MessageBox MB_OKCANCEL|MB_SETFOREGROUND "Select Python Path" "Choose the Python path you want to use:"
    ${If} $0 == "CA"
        Abort
    ${EndIf}

    FileWrite "${INSTDIR}\app.config" "PYTHONPATH=xxxx"
;SectionEnd