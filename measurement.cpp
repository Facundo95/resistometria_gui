#include "measurement.h"
#include <cmath>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#ifdef ENABLE_IEEE_HARDWARE
#include "IEEE-C.H"
#endif

Measurement::Measurement()
    : active(false), step_count(1), hardware_connected(false), last_connection_success(false),
      sample_interval_seconds(1.0), configured_current_amp(1e-1), elapsed_time_seconds(0.0) {}

void Measurement::set_acquisition_params(double interval_seconds, double current_amp) {
    sample_interval_seconds = (interval_seconds > 0.0) ? interval_seconds : 1.0;

    if (current_amp < 0.0) current_amp = -current_amp;
    configured_current_amp = (current_amp > 0.0) ? current_amp : 1e-1;
}

bool Measurement::connect_hardware() {
#ifdef ENABLE_IEEE_HARDWARE
    long int status = 0;

    int tipo_de_interface = gpib_board_present();
    std::cout << "Tipo de KM-488 = " << tipo_de_interface << "\n";
    if (tipo_de_interface == 0) {
        last_status_message = "No se detecto la interfaz GPIB KM-488. Prueba conectar los equipos y reiniciar la computadora.";
        std::cerr << last_status_message << "\n";
        return false;
    }

    initialize(21, 0);

    if (!listener_present(7)) {
        last_status_message = "Nanovoltimetro (addr 7) no conectado.";
        std::cerr << last_status_message << "\n";
        return false;
    }
    if (!listener_present(12)) {
        last_status_message = "Fuente de corriente (addr 12) no conectada.";
        std::cerr << last_status_message << "\n";
        return false;
    }
    if (!listener_present(16)) {
        last_status_message = "Multimetro (addr 16) no conectado.";
        std::cerr << last_status_message << "\n";
        return false;
    }

    send(7, "*RST", &status);
    send(12, "*RST", &status);
    send(16, "*RST", &status);

    send(7, "SENS:FUNC 'VOLT'", &status);
    send(16, ":SENS:FUNC 'TEMP'", &status);
    send(16, ":SENS:TEMP:TC:TYPE k", &status);
    send(16, ":SENS:TEMP:TC:RJUN:RSEL SIM", &status);
    send(16, ":SENS:TEMP:TC:RJUN:SIM 0", &status);
    send(12, "outp on", &status);

    last_status_message = "Dispositivos conectados correctamente.";

    return true;
#else
    last_status_message = "Modo simulado activo (ENABLE_IEEE_HARDWARE no definido).";
    std::cout << last_status_message << "\n";
    return true;
#endif
}

bool Measurement::start(const char* filename) {
    last_connection_success = false;

    if (!hardware_connected) {
        hardware_connected = connect_hardware();
        last_connection_success = hardware_connected;
        if (!hardware_connected) return false;
    } else {
        last_connection_success = true;
        if (last_status_message.empty()) {
            last_status_message = "Dispositivos conectados correctamente.";
        }
    }

    salida.open(filename);
    if (salida.is_open()) {
        salida << "N,time(s),temp(ºC),current(mA),voltage(mV),resistance(Ohms)\n";
        active = true;
        step_count = 1;
        elapsed_time_seconds = 0.0;
        return true;
    }

    last_status_message = "No se pudo abrir el archivo de salida.";
    return false;
}

bool Measurement::resume() {
    if (!hardware_connected) {
        hardware_connected = connect_hardware();
        last_connection_success = hardware_connected;
        if (!hardware_connected) return false;
    }

    if (!salida.is_open()) {
        last_status_message = "No hay un archivo de salida abierto para continuar.";
        return false;
    }

    active = true;
    last_connection_success = true;
    return true;
}

void Measurement::pause() {
    active = false;
}

void Measurement::stop() {
    active = false;

#ifdef ENABLE_IEEE_HARDWARE
    long int status = 0;
    send(12, "sour:clear:imm", &status);
#endif

    if (salida.is_open()) salida.close();
}

MeasurementData Measurement::perform_measurement_cycle() {
    MeasurementData d;
    d.n = step_count++;
    elapsed_time_seconds += sample_interval_seconds;
    d.time = elapsed_time_seconds;

#ifdef ENABLE_IEEE_HARDWARE
    long int status = 0;
    int len = 0;

    char order[64];
    char voltage_ch[32];
    char temp_ch[32];
    double voltage_pos = 0.0;
    double voltage_neg = 0.0;

    std::snprintf(order, sizeof(order), "sour:curr:ampl %.9g", configured_current_amp);
    send(12, order, &status);
    send(7, "sens:chan 1; :read?", &status);
    send(16, ":read?", &status);
    enter(voltage_ch, 32, &len, 7, &status);
    enter(temp_ch, 32, &len, 16, &status);
    voltage_pos = std::atof(voltage_ch);

    std::snprintf(order, sizeof(order), "sour:curr:ampl %.9g", -configured_current_amp);
    send(12, order, &status);
    send(7, "sens:chan 1; :read?", &status);
    send(16, ":read?", &status);
    enter(voltage_ch, 32, &len, 7, &status);
    enter(temp_ch, 32, &len, 16, &status);
    voltage_neg = std::atof(voltage_ch);

    d.current = configured_current_amp;
    d.temp = std::atof(temp_ch);
    d.voltage = (voltage_neg - voltage_pos) / 2.0;
    d.resistance = std::fabs(d.voltage / configured_current_amp);
#endif

    return d;
}

MeasurementData Measurement::nextStep() {
    MeasurementData d = perform_measurement_cycle();

    if (salida.is_open()) {
        salida << d.n << "," << d.time << "," << d.temp << "," 
               << d.current << "," << d.voltage << "," << d.resistance << "\n";
        salida.flush();
    }
    return d;
}