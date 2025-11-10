*** Settings ***
Library   String
Library   SerialLibrary

*** Variables ***
${com}        COM10
${baud}       115200
${board}      nRF
${ok_seq}     T000120
${err_seq}    T001067
${ok_resp}    80X
${err_resp}   -6X

*** Test Cases ***
Connect Serial
    Log To Console  Connecting to ${board}
    Add Port  ${com}  baudrate=${baud}  encoding=ascii
    Port Should Be Open  ${com}
    Reset Input Buffer
    Reset Output Buffer

Valid Time String
    Reset Input Buffer
    Reset Output Buffer
    Write Data   ${ok_seq}   encoding=ascii
    Sleep   0.5s
    ${read}=   Read Until   terminator=58   encoding=ascii   timeout=2s
    Log To Console   Received: ${read}
    Should Contain   ${read}    ${ok_resp}

Invalid Time String
    Reset Input Buffer
    Reset Output Buffer
    Write Data   ${err_seq}   encoding=ascii
    Sleep   0.5s
    ${read}=   Read Until   terminator=58   encoding=ascii   timeout=2s
    Log To Console   Received: ${read}
    Should Contain   ${read}    ${err_resp}

Disconnect Serial
    Log To Console  Disconnecting ${board}
    [TearDown]  Delete Port  ${com}
