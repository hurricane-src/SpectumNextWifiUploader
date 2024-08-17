
; ESP_STATE_MACHINE_CHAR=0 char jump-if-not
; ESP_STATE_MACHINE_EXPECTED=1 n expected-chars
; ESP_STATE_MACHINE_STRING=2 parsed-string-address
; ESP_STATE_MACHINE_INT=3 parsed-int16-address
; ESP_STATE_MACHINE_CALL=4 address
; ESP_STATE_MACHINE_SET_VALUE=5 address set-int8
; ESP_STATE_MACHINE_END=6
; ESP_STATE_MACHINE_ERROR=7
; ESP_STATE_MACHINE_CRLF=8
; ESP_STATE_MACHINE_SKIP_UNTIL=9 skip-until-char
; ESP_STATE_MACHINE_BREAKPOINT=10

esp_state_machine:

ok_parse:
    db ESP_STATE_MACHINE_CHAR, 'O'
        dw error_parse
    db ESP_STATE_MACHINE_EXPECTED, 1, 'K'
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_SET_VALUE
        dw esp_state_ok
        db 1
    db ESP_STATE_MACHINE_END

error_parse:
    db ESP_STATE_MACHINE_CHAR, 'E'
        dw wifi_parse
    db ESP_STATE_MACHINE_EXPECTED, 4, 'R', 'R', 'O', 'R'
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_SET_VALUE
        dw esp_state_error
        db 1
    db ESP_STATE_MACHINE_END

wifi_parse:
    db ESP_STATE_MACHINE_CHAR, 'W'
        dw plus_parse
    db ESP_STATE_MACHINE_EXPECTED, 4, 'I', 'F', 'I', ' '
wifi_connected_parse:
    db ESP_STATE_MACHINE_CHAR, 'C'
        dw wifi_disconnected_parse
    db ESP_STATE_MACHINE_EXPECTED, 8, 'O', 'N', 'N', 'E', 'C', 'T', 'E', 'D'
    db ESP_STATE_MACHINE_SET_VALUE
        dw wifi_connected_flag
        db 1
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_END
wifi_disconnected_parse:
    db ESP_STATE_MACHINE_CHAR, 'D'
        dw wifi_got_ip_parse
    db ESP_STATE_MACHINE_EXPECTED, 11, 'I', 'S', 'C', 'O', 'N', 'N', 'E', 'C', 'T', 'E', 'D'
    db ESP_STATE_MACHINE_SET_VALUE
        dw wifi_connected_flag
        db 0
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_END
wifi_got_ip_parse:
    db ESP_STATE_MACHINE_CHAR, 'G'
        dw plus_parse
    db ESP_STATE_MACHINE_EXPECTED, 5, 'O', 'T', ' ', 'I', 'P'
    db ESP_STATE_MACHINE_SET_VALUE
        dw wifi_ip_obtained
        db 1
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_END

plus_parse:
    db ESP_STATE_MACHINE_CHAR, '+'
        dw no_change_parse
plus_c_parse:
    db ESP_STATE_MACHINE_CHAR, 'C'
        dw plus_ipd_parse
plus_cifsr_parse:
    db ESP_STATE_MACHINE_CHAR, 'I'
        dw plus_cwlap_parse
    db ESP_STATE_MACHINE_EXPECTED, 7, 'F', 'S', 'R', ':', 'S', 'T', 'A'
plus_if_ip_parse:
    db ESP_STATE_MACHINE_CHAR, 'I'
        dw plus_if_mac_parse
    db ESP_STATE_MACHINE_EXPECTED, 2, 'P', ','
    db ESP_STATE_MACHINE_STRING
        dw wifi_ip_address
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_END
plus_if_mac_parse:
    db ESP_STATE_MACHINE_CHAR, 'M'
        dw error_terminator
    db ESP_STATE_MACHINE_EXPECTED, 3, 'A', 'C', ','
    db ESP_STATE_MACHINE_STRING
        dw wifi_mac_address
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_END
plus_cwlap_parse: ; only expexts it to be +CWLAP 
    ;db ESP_STATE_MACHINE_BREAKPOINT
    db ESP_STATE_MACHINE_EXPECTED, 6,'W','L','A','P',':','('
    ; then one digit for the mode (0=open,wep,wpa_psk,wpa2_psk,wpa_wpa2_psk)
    db ESP_STATE_MACHINE_INT
        dw wifi_ap_ecn
    db ESP_STATE_MACHINE_STRING
        dw wifi_ap_ssid
    db ESP_STATE_MACHINE_SKIP_UNTIL, ')'
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_END

plus_ipd_parse:
    db ESP_STATE_MACHINE_CHAR, 'I'
        dw error_terminator
    db ESP_STATE_MACHINE_EXPECTED, 3, 'P', 'D', ','
    db ESP_STATE_MACHINE_INT
        dw connection_current
    db ESP_STATE_MACHINE_INT
        dw esp_recv_count
    db ESP_STATE_MACHINE_CALL
        dw client_execute_command
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_END
    
no_change_parse:
    db ESP_STATE_MACHINE_CHAR, 'n'
        dw crlf_parse
    db ESP_STATE_MACHINE_EXPECTED, 8, 'o', ' ', 'c', 'h', 'a', 'n', 'g', 'e'
    db ESP_STATE_MACHINE_CRLF
    ;db ESP_STATE_MACHINE_SET_VALUE
    ;    dw esp_state_ok
    ;    db 1
    db ESP_STATE_MACHINE_END

crlf_parse:
    db ESP_STATE_MACHINE_CHAR, 13
        dw connection_parse
    db ESP_STATE_MACHINE_EXPECTED, 1, 10
    db ESP_STATE_MACHINE_END

connection_parse:
    db ESP_STATE_MACHINE_INT
        dw connection_current
connection_connect_parse:
    db ESP_STATE_MACHINE_CHAR, 'C'
        dw connection_disconnect_parse
    db ESP_STATE_MACHINE_EXPECTED, 6, 'O', 'N', 'N', 'E', 'C', 'T'
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_END
connection_disconnect_parse:
    db ESP_STATE_MACHINE_CHAR, 'D'
        dw error_terminator
    db ESP_STATE_MACHINE_EXPECTED, 10, 'D', 'I', 'S', 'C', 'O', 'N', 'N', 'E', 'C', 'T'
    db ESP_STATE_MACHINE_CRLF
    db ESP_STATE_MACHINE_END

error_terminator:
    db ESP_STATE_MACHINE_ERROR
