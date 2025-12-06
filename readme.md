# Dispositivo de Detección de Caídas con ESP32 (VERSIÓN BÁSICA)

Este proyecto consiste en un dispositivo portátil desarrollado con la placa **ESP32** cuya finalidad es identificar caídas de manera automática mediante la lectura de datos de un sensor inercial. El sistema analiza los cambios bruscos de aceleración (jerk) para determinar si se ha producido un evento compatible con una caída real. Cuando esto ocurre, se activa una alarma sonora para alertar a las personas cercanas. El usuario puede cancelar la alarma mediante un pulsador especialmente dedicado a esta función. Además, el dispositivo incorpora conectividad inalámbrica y capacidad para enviar mensajes de emergencia a contactos previamente almacenados.

---

## Características Principales

- **Detección de caídas en tiempo real:**  
  El sistema monitoriza constantemente el movimiento del usuario utilizando el sensor **MPU6050**. Cuando se detecta un cambio repentino en la aceleración que supera un umbral determinado, el firmware interpreta dicho evento como una posible caída.

- **Alarma acústica integrada:**  
  Ante una caída confirmada, el dispositivo activa un **zumbador (buzzer)** que emite un sonido continuo para pedir ayuda o llamar la atención de otras personas.

- **Botón de cancelación con interrupción:**  
  Un pulsador físico permite desactivar la alarma. Su manejo genera una interrupción que detiene el zumbador de forma inmediata, evitando falsas alarmas o permitiendo silenciar la alerta tras verificar la seguridad del usuario.

- **Gestor de WiFi incorporado:**  
  El ESP32 integra un sistema de configuración inalámbrica que facilita la conexión a redes WiFi sin necesidad de modificar el código. Esta función permite habilitar futuras integraciones de comunicación remota.

- **Envío de SMS de emergencia (SOS):**  
  En caso de caída, el dispositivo puede enviar mensajes SMS a los contactos registrados utilizando servicios basados en la nube. De esta manera, un familiar o persona designada recibe un aviso inmediato solicitando asistencia.

---

## 🧩 Componentes de Hardware Utilizados

<p align="center">
<img src="(tu_imagen_aqui).jpg" height="500" width="500">
</p>
<br>

- **ESP32 DevKit:**  
  Microcontrolador principal que ejecuta el firmware, gestiona la comunicación WiFi y controla los periféricos conectados.

- **Sensor MPU6050 (Acelerómetro + Giroscopio):**  
  Proporciona lecturas de aceleración y velocidad angular. El análisis del jerk (variación repentina de la aceleración) permite identificar patrones asociados a caídas.

- **Zumbador Piezoeléctrico:**  
  Genera una señal sonora para advertir de la caída.

- **Pulsador de Cancelación:**  
  Botón destinado a detener la alarma mediante una interrupción externa.

- **Conectividad WiFi integrada:**  
  Utilizada para enviar datos remotos, acceder a servicios cloud y transmitir mensajes SMS a contactos de emergencia.

- **Memoria flash interna (SPIFFS):**  
  Permite almacenar información como credenciales WiFi, lista de contactos y configuraciones específicas del sistema.

Todos los componentes trabajan conjuntamente para proporcionar un sistema completo, fiable y fácilmente ampliable.

---

## ⚙️ Funcionamiento del Sistema

<p align="center">
<img src="(tu_otra_imagen_aqui).jpg" height="700" width="450">
</p>
<br>

1. **Lectura del Movimiento:**  
   El sensor MPU6050 proporciona valores de aceleración y rotación del usuario en tiempo real.

2. **Detección de Caída:**  
   El firmware evalúa la variación de aceleración (jerk). Cuando esta supera un umbral establecido, se interpreta como una posible caída.

3. **Activación del Buzzer:**  
   Si el evento cumple las condiciones de detección, el dispositivo activa el zumbador.

4. **Envío de Mensaje SOS:**  
   El sistema obtiene las credenciales almacenadas en la memoria y envía un mensaje de auxilio a los contactos registrados.

5. **Cancelación Manual:**  
   El usuario puede pulsar el botón dedicado para detener la alarma y silenciar el zumbador.

6. **Conexión WiFi:**  
   Para la configuración inicial, el dispositivo habilita un modo de gestión WiFi que permite establecer la red desde un móvil o PC sin reprogramar la placa.

---
## Mejoras Implementadas en esta Versión (PROXIMAMENTE)

Además de las funciones básicas de detección de caídas y alerta sonora, esta versión del proyecto incorpora una serie de mejoras que lo diferencian notablemente de un detector convencional, orientándolo hacia un sistema inteligente de asistencia real.
**1. Notificaciones Automáticas a través de WhatsApp**
Cuando se detecta una caída, el dispositivo no solo activa el buzzer, sino que envía un mensaje de alerta mediante WhatsApp a un contacto de emergencia previamente configurado.
Esta funcionalidad se implementa mediante la WhatsApp Cloud API, lo que permite:

-Enviar mensajes instantáneos sin necesidad de tarjeta SIM
-Contacto directo con familiares o cuidadores
-Confirmación de recepción y posibilidad de respuesta
-Uso de la aplicación más extendida entre personas mayores y adultos

Con esta mejora, el sistema deja de ser un simple detector y se convierte en un dispositivo de comunicación inteligente orientado a la asistencia remota.
**2. Registro de Eventos en Memoria (Histórico de Caídas)**
El dispositivo almacena cada caída detectada en la memoria interna (SPIFFS), registrando datos como:
fecha y hora del incidente,
número de eventos detectados,
estado del usuario tras la caída.

Esto permite al cuidador o técnico analizar el historial y evaluar la evolución del usuario, añadiendo una dimensión temporal y diagnóstica al sistema.
**3. Sistema de Indicadores LED Inteligentes**
El LED del dispositivo ya no actúa únicamente como un simple encendido/apagado: ahora representa estados del sistema mediante colores distintos:
Estado del sistema	Color LED
-En funcionamiento normal	Verde
-Posible caída detectada	Amarillo
-Caída confirmada	Rojo
-Alarma cancelada	Azul

Este lenguaje visual facilita la interacción del usuario con el dispositivo y mejora la comprensión del estado operativo sin necesidad de pantalla.
**4. Extensibilidad hacia Visión Artificial**
El sistema ha sido diseñado para admitir módulos de visión artificial en versiones futuras. Esto permite:
-Analizar la postura del usuario mediante cámaras externas
-Confirmar visualmente si una caída ha ocurrido realmente
-Reducir falsos positivos mediante análisis corporal
-Complementar los datos del acelerómetro con información contextual

Esta mejora convierte el proyecto en una plataforma híbrida capaz de evolucionar hacia soluciones asistenciales más complejas, como las que se utilizan en sistemas de teleasistencia profesional.
 **Visión Futura del Sistema**
La arquitectura actual no solo detecta caídas: está preparada para convertirse en un ecosistema de asistencia inteligente.
La integración de algoritmos predictivos, comunicación avanzada y módulos perceptivos sitúan este dispositivo en la línea de los desarrollos tecnológicos que definen la Industria 5.0, donde el usuario y la tecnología conviven de manera coordinada.

<p align="center">
<img src="(tu_otra_imagen_aqui).jpg" height="700" width="450">
</p>
<br>

##  Cómo Empezar

1. Ensamblar el hardware siguiendo la disposición recomendada.
2. Cargar el firmware en la placa ESP32 mediante Arduino IDE o PlatformIO.
3. Configurar la red WiFi utilizando el gestor integrado.
4. Registrar los contactos de emergencia.
5. Colocar el dispositivo sobre el usuario.
6. El sistema comenzará a monitorizar los movimientos automáticamente.

En caso de caída, el zumbador se activará y los contactos recibirán un mensaje solicitando ayuda.

---

##  AutorES

**COLOCAR NOMBRES COMPLETOS  
Ingeniería en Robótica e Inteligencia Artificial**

---

