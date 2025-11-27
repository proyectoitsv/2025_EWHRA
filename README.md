# 🧠 EWHRA  
### *Electroencephalographic Wave Helmet to Regulating Attention*  
**Sistema EEG portátil de un canal para estimar el nivel de atención mediante análisis de amplitud en tiempo real.**

---

## 📌 1. ¿Qué es EWHRA?

**EWHRA** es un dispositivo portátil basado en EEG que adquiere, filtra y digitaliza la actividad cerebral mediante un **sistema de un canal**.  
La señal procesada es enviada por **Bluetooth Low Energy (BLE)** a una aplicación móvil que muestra:

- Una **barra de nivel** proporcional al valor del ADC.  
- Un estado simplificado: **ALTO**, **MEDIO** o **BAJO**.  
- El valor analógico procesado en tiempo real.

Este prototipo tiene fines **educativos, experimentales y de investigación**, y sirve para comprender de manera práctica cómo se adquieren y procesan señales biológicas reales.

> ⚠️ **Advertencia:** Este sistema no es un equipo médico y no debe utilizarse para diagnóstico o tratamiento.

---

## 🎯 2. Objetivos del Proyecto

### **Objetivo General**
Detectar variaciones en la amplitud de ondas beta (13–30 Hz) y representar el nivel de atención en una aplicación móvil mediante BLE.

### **Objetivos Específicos**
- Captar actividad EEG con electrodos de oro.  
- Amplificar y filtrar la señal mediante un procesamiento analógico adecuado.  
- Digitalizar la señal con el ADC del ESP32-C3.  
- Transmitir la lectura por BLE en tiempo real.  
- Representar visualmente el nivel de atención en la app mediante indicadores simples.

---

## ⚙️ 3. ¿Cómo funciona EWHRA?

El sistema opera mediante **tres etapas principales**:

---

### **1) Adquisición de señal**
- Se emplean **tres electrodos EEG** (activo, referencia y masa).  
- La señal captada está en el rango de microvoltios, por lo que requiere amplificación precisa.

---

### **2) Procesamiento analógico**
Incluye tres subetapas:

1. **Amplificación diferencial — INA129P**  
   Amplifica la señal EEG manteniendo un elevado rechazo al modo común (CMRR).

2. **Filtrado pasabanda — TL084CN (13–30 Hz)**  
   Aísla las ondas beta y reduce ruido fuera del rango de interés.

3. **Ganancia final**  
   Ajusta la amplitud total para que el ADC del ESP32-C3 pueda digitalizarla sin saturación.

---

### **3) Procesamiento digital**
El **ESP32-C3 SuperMini** se encarga de:

1. Digitalizar la señal mediante su **ADC de 12 bits** (0–4095).  
2. Calcular una medida representativa (amplitud o RMS).  
3. Enviar los valores por **BLE** a la aplicación móvil.  

En la app, los valores se interpretan así:

| Estado | Rango del ADC |
|--------|----------------|
| **ALTO** | > 66% |
| **MEDIO** | 33–66% |
| **BAJO** | < 33% |

La app muestra una barra y un mensaje según el nivel.

---

## 🧩 4. Hardware del Sistema

| Componente | Función | Motivo |
|------------|---------|--------|
| **Electrodos de oro** | Captura EEG | Buen contacto, baja impedancia |
| **INA129P** | Amplificador de instrumentación | Alto CMRR, ideal para EEG |
| **TL084CN** | Filtro pasabanda + ganancia | Bajo ruido y buena respuesta AC |
| **ESP32-C3 SuperMini** | ADC + BLE + CPU | Tamaño reducido, BLE integrado |
| **7805** | Regulación | 5V estables |
| **Batería 3.7–5 V** | Fuente aislada | Menor interferencia electromagnética |

---

## 🧱 5. Arquitectura del Sistema

```mermaid
flowchart TD
    A[Electrodos EEG] --> B[Amplificador INA129P]
    B --> C[Filtro Pasabanda 13-30 Hz - TL084CN]
    C --> D[Ganancia Final - TL084CN]
    D --> E[ESP32-C3 SuperMini - ADC 12 bits]
    E --> F[Bluetooth Low Energy]
    F --> G[Aplicación Móvil - App Inventor]
    G --> H[Visualización: Barra + Estado]
