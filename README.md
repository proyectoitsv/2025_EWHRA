🧠 EWHRA
Electroencephalographic Wave Helmet to Regulating Attention

Sistema EEG portátil de un canal para estimar el nivel de atención mediante análisis de amplitud en tiempo real.

📌 1. ¿Qué es EWHRA?

EWHRA es un dispositivo portátil basado en EEG que adquiere, filtra y digitaliza la actividad cerebral mediante un sistema de un canal. La señal procesada es enviada por BLE a una aplicación móvil que muestra:

Una barra de nivel proporcional al valor del ADC.

Un estado simplificado: ALTO, MEDIO o BAJO.

El valor analógico procesado en tiempo real.

Este prototipo tiene fines educativos, experimentales y de investigación, y busca explorar la adquisición de señales biológicas reales mediante hardware accesible y procesamiento básico.

⚠️ No es un dispositivo médico, ni está diseñado para diagnóstico o tratamiento.

🎯 2. Objetivos del Proyecto
Objetivo General

Detectar variaciones en la amplitud de ondas beta (13–30 Hz) y representar un nivel de atención en una app móvil conectada mediante Bluetooth Low Energy.

Objetivos Específicos

Captar actividad EEG con electrodos de oro.

Amplificar y filtrar la señal mediante un sistema analógico adecuado para EEG.

Digitalizar la señal con el ADC del ESP32-C3.

Transmitir la lectura por BLE.

Mostrar el nivel de atención en una interfaz gráfica simple e intuitiva.

⚙️ 3. ¿Cómo funciona EWHRA?

El funcionamiento se organiza en tres etapas principales:

1) Adquisición de señal

Se utilizan tres electrodos (activo, referencia y masa).

La señal EEG se encuentra en el rango de microvoltios, por lo que requiere amplificación precisa.

2) Procesamiento analógico

INA129P: amplifica diferencialmente la señal captada.

Filtro pasabanda 13–30 Hz (TL084CN): elimina componentes fuera del rango beta.

Etapa de ganancia final: ajusta la amplitud para el ADC del ESP32-C3.

Este bloque se encarga de entregar una señal limpia, estable y dentro del rango permitido.

3) Procesamiento digital

El ESP32-C3:

Toma muestras mediante su ADC de 12 bits (0–4095).

Calcula un valor representativo (ej. RMS o amplitud directa).

Envía los datos por BLE a la aplicación móvil.

En la app, el valor se interpreta como:

Estado	Rango del ADC
ALTO	> 66%
MEDIO	33–66%
BAJO	< 33%
🧩 4. Hardware del Sistema
Componente	Rol en el sistema	Motivo de uso
Electrodos Au	Captura de señal EEG	Baja impedancia, buen contacto
INA129P	Amplificación diferencial	Alta precisión y CMRR ideal para EEG
TL084CN	Filtro pasabanda + ganancia	Amplificadores operacionales de bajo ruido
ESP32-C3 SuperMini	ADC + BLE + procesamiento	Tamaño reducido y conectividad integrada
7805	Regulación	Alimentación estable para el módulo
Batería 3.7–5 V	Fuente aislada	Reduce interferencias electromagnéticas
🧱 5. Arquitectura del Sistema
flowchart TD
    A[Electrodos EEG] --> B[Amplificador INA129P]
    B --> C[Filtro Pasabanda 13-30 Hz - TL084CN]
    C --> D[Ganancia Final - TL084CN]
    D --> E[ESP32-C3 SuperMini - ADC 12 bits]
    E --> F[Bluetooth Low Energy]
    F --> G[Aplicación Móvil - App Inventor]
    G --> H[Visualización: Barra + Estado]
