# 🧠 EWHRA  
### Electroencephalographic Wave Helmet to Regulating Attention  
Sistema EEG portátil de un canal para estimar el nivel de atención mediante análisis simple de amplitud de señal.

---

## 1. ¿Qué es EWHRA?

**EWHRA** es un sistema portátil que capta ondas cerebrales mediante **electrodos EEG**, las filtra y amplifica a través de un circuito analógico, y luego las digitaliza con un **ESP32-C3 SuperMini**.

El objetivo es que la señal resultante pueda utilizarse para **estimar el nivel de atención del usuario**, mostrando en una aplicación móvil indicadores visuales simples:

- Una **barra de nivel** basada en el valor del ADC.  
- Un estado categorizado como: **ALTO**, **MEDIO**, **BAJO**.  
- Conexión BLE con el dispositivo para ver el valor en tiempo real.

No es un equipo médico, sino un **prototipo educativo y experimental** para comprender cómo se adquiere y procesa una señal EEG real.

---

## 2. Objetivos del Proyecto

### Objetivo General
Detectar en tiempo real cambios en la amplitud de una señal EEG filtrada y mostrar el estado de atención del usuario en una interfaz móvil mediante Bluetooth Low Energy (BLE).

### Objetivos Específicos
- Captar señales EEG mediante electrodos de oro.  
- Amplificar y filtrar la señal para aislar el rango 13–30 Hz (ondas beta).  
- Digitalizar la señal con el ADC del ESP32-C3.  
- Transmitir el valor de la señal vía BLE.  
- Representar visualmente el nivel en una app mediante barras y mensajes.  

---

## 3. ¿Cómo funciona EWHRA? 

El flujo del sistema se divide en **tres etapas principales**:

### **1) Adquisición de señal**
- Tres electrodos capturan la actividad eléctrica del cuero cabelludo.  
- Estas señales son muy débiles (microvoltios) y requieren amplificación especializada.

### **2) Procesamiento analógico**
- Un **amplificador de instrumentación INA129P** eleva la señal EEG manteniendo relación precisa entre electrodos.  
- Un **filtro pasa banda activo (13–30 Hz)** atenúa todo lo que no sean ondas beta.  
- Una etapa de amplificación final ajusta la amplitud para que el ADC pueda leerla.

### **3) Procesamiento digital**
- El **ESP32-C3** toma muestras de la señal mediante su ADC.  
- El valor analógico se convierte en un número digital de **0 a 4095** (resolución de 12 bits).  
- Ese valor se envía a una app por **Bluetooth Low Energy (BLE)**.

La app interpreta este número como:

| Estado | Rango (porcentaje del ADC) |
|--------|-----------------------------|
| **ALTO** | > 66% |
| **MEDIO** | 33–66% |
| **BAJO** | < 33% |

Esto se muestra mediante una **barra de nivel** y un **mensaje de estado**.

---

## 4. Hardware del Sistema

### Componentes principales

| Componente | Función | Motivo |
|------------|---------|--------|
| **Electrodos Au EEG** | Detección de señal cerebral | Buena conductividad y bajo ruido |
| **INA129P** | Amplificador de instrumentación | Alta precisión, ideal para EEG |
| **TL084CN** | Filtro y amplificación activa | Configurable para 13–30 Hz |
| **ESP32-C3 SuperMini** | ADC + BLE + procesamiento | Tamaño reducido y BLE integrado |
| **7805** | Regulador lineal | Alimentación estable a 5V |
| **Batería 3.7V / 5V** | Fuente aislada | Reduce interferencias EM |

## Arquitectura del Sistema 

```mermaid
flowchart TD
    A[Electrodos EEG] --> B[Amplificador de Instrumentación INA129P]
    B --> C[Filtro Pasabanda 13-30 Hz - TL084CN]
    C --> D[Amplificador Final - TL084CN]
    D --> E[ESP32-C3 SuperMini - ADC 12 bits]
    E --> F[Bluetooth Low Energy - Servicio Único]
    F --> G[Aplicación Móvil - App Inventor]
    G --> H[Barra de Nivel + Estado Alto/Medio/Bajo]

 ---