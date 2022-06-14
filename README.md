# Bomba-Peristaltica
Bomba peristáltica

el código del arduino está basado en https://www.thingiverse.com/thing:2619479/files  y modificado según mis necesidades, en particular, la forma de calibración. Nosotros no necesitamos un volumen exacto y además la consistencia de la comida varía bastante en función de la temperatura, por lo que el ajuste para que dispense un cierto volumen puede cambiar bastante en función de la temperatura de la comida. Por eso implementé un calibrado “a ojo” que es más que suficiente para mis necesidades.
Hay dos puntos críticos a tener en cuenta:
-material utilizado para imprimir las piezas: el PLA se ablanda a 65ºC aproximadamente. Las primeras pruebas fueron impresas con ese material y no funcionaron bien, en particular, el rotor. Por eso rediseñé el rotor y lo imprimí en PETG, que resiste bien hasta 85C. El resto de las piezas no fueron reimpresas en ese material. Aparentemente, funcionan bien en PLA.
-temperatura de la comida: cocinamos la comida en una olla a baño maría con termostato fijo a 95C. A esa temperatura, el PLA se ablanda. El rotor es particularmente sensible a este efecto porque se deforma el orificio donde encaja el motor. Por eso, lo reimprimimos con PETG y además bajamos la temperatura a 70C para distribuirla. Es decir, cocinamos a 95 y luego bajamos a 70C. De esa manera, logramos distribuir 4 litros de comida sin ningún inconveniente. Por otra parte, es importante que se mantenga la temperatura de la comida ya que si se calibra a una temperatura para que impulse un cierto volumen de comida en cada ciclo y luego la comida se enfría, el volumen dispensado va a ser menor. La solución es recalibrar (es muy fácil de hacer) cuando sea necesario o mantener la comida a una temperatura fija.

El modelo inicial del rotor fue copiado de https://www.thingiverse.com/thing:3148717 pero luego modificado y es el que te envío. Recomienda imprimirlo con infill 100%. Yo recomiendo usar PETG, al menos para el rotor.
Mi idea era hacer una bomba que pudiera usarse por dos laboratorios diferentes, el mío, que trabaja con moscas y otro de gusanos (por eso vas a ver que tiene dos programas denominados “moscas” y “gusanos”. Se puede cambiar el nombre en las líneas 161 y 162 del código). En este último caso, se necesita distribuir un medio de cultivo estéril y por lo tanto, necesitan esterilizar la manguera luego de cada uso. Para que fuera sencillo cambiar la manguera luego de cada uso diseñé las piezas que adjunto. Pero luego no funcionaron tal como esperaba, ya que para evitar que el motor se trabe es necesario que las piezas tengan algo de juego, que puedan moverse levemente. La solución que encontré fue hacerles un agujero con un taladro y atravesar un tornillo para mantener unidas las dos piezas entre sí y dejar que puedan moverse. En el modelo que te adjunto ya incluí el par de orificios. Este diseño se parece bastante en su funcionamiento a aquel del que copié el rotor. La manguera puede aun cambiarse con relativa facilidad. Si sólo vas a usarlo para comida de moscas, quizás pueda ser más práctico imprimir directamente el modelo completo del que saqué el rotor, que también incluye una carcasa para alojar el motor y el controlador. El cambio de la manguera parece más trabajoso en ese modelo, pero eso no sería un inconveniente si no es necesario esterilizarla luego de cada uso ya que puede lavarse el interior pasando agua caliente.

Envío también el circuito eléctrico. Hay tres cosas que son opcionales. Una es la activación con un pedal. La idea era llenar los tubos de a uno activando la bomba con un pedal, pero encontramos que la alternativa de generar un programa con un número de ciclos (=tubos) y una pausa entre ellos era mejor. La otra característica opcional son los leds que indican el modo de funcionamiento. La última es el ventilador para enfriar el driver del motor y el motor mismo. Las especificaciones del driver indican que puede llegar a ser necesario refrigerarlo. Como es barato, preferí agregarlo, pero me parece que funcionaría igual de bien sin él.
 

1- motor paso a paso (stepper motor) 1.5A Alto Torque 4.2kg 17HS4401S   x1 https://articulo.mercadolibre.com.ar/MLA-899670687-nema-17-motor-paso-a-paso-18-alto-t-15a-usongshine-_JM
2- ruleman 605zz	x9	https://articulo.mercadolibre.com.ar/MLA-915854069-rodamiento-ruleman-605-zz-x20-unidades-_JM
3- driver A4899		x1 https://www.todomicro.com.ar/motores-y-drivers-motores/288-modulo-a4988-pololu-driver-motor-paso-a-paso.html
4- arduino promini 3.3V (se puede usar también de 5V o un arduino UNO)	x1
5- Fuente 12V 2A	x1
6- Regulador step down MP1584 	x1
7- Display LCD 16x2 con adaptador I2C	x1
8- Encoder Rotativo Ky-040 	x1
9- LEDs 3mm (opcional)	x3
10- condensador de 100uF	x1
11- resistencias 220 Ohm, 10kOhm
12- ventilador de 6cm (opcional)	x1
13- caja plástica: se puede imprimir. Usé una caja estanca de aprox 20x30 cm
14- la manguera es de silicona diámetro ext 10mm, diámetro int 6mm (paredes 2mm). 
15- tornillos, tuercas…

AJUSTES de corriente máxima para el driver del motor
Este punto es muy importante. Hay que seguir las instrucciones que se encuentran aquí: https://www.pololu.com/product/1182
El módulo que conseguí no es el original y no tiene las resistencias que se indican en ese instructivo. De todas maneras, el método de ajuste es el mismo. A continuación, pego mis notas respecto de esto:
Motor 17HS4401S
según el que lo vendió la intensidad máxima es 1.5A
para ajustar el A4988 Imax = Vref/8*resistencia limitadora
Imax es 1.5
la resistencia es R100 0.1Ohm o 100 mOhm
Vref= 1.2V

Uso el driver en modo quarter steps con un voltaje lógico de 5V.

Ajustando el potenciómetro del módulo al máximo, el máximo valor Vref alcanzado es de 1.19V, que es el Vref con el que se alcanza el Imax de 1.5 A
es decir, que quedó en el límite superior.



Archivos para impresión 3D:
1)	Rotor-A.stl (PETG)
2)	Rotor-B.stl (PETG)
3) piezasCubreRotor-modificado-inferior.stl
4) piezasCubreRotor-modificado-superior.stl

Funcionamiento:
Al encenderla, aparece en el display un menú con 11 opciones. Girando el control en sentido horario o antihorario se cambia de menú. Presionando la misma perilla, se entra en el menú y pueden seleccionarse opciones. Presionando nuevamente, se sale del menú.
En cualquier momento, si se mantiene presionado el selector comenzará a funcionar la bomba a una velocidad fija (no necesariamente la misma que la seleccionada en el menú 3) en forma continua hasta que se suelte. Eso permite llenar la manguera.
Menú 
1: Start--> inicia el programa (dosifica/bombea/calibra). Presionando nuevamente, interrumpe la ejecución al finalizar el ciclo actual.
2: Tiempo --> permite establecer el tiempo de llenado deseado. Calcula automáticamente las RPM. Si RPM es mayor que 200, ajusta el tiempo para no sobrepasar 200.
3: Velocidad --> permite ajustar RPM, calcula automáticamente el tiempo de llenado.
4: Ciclos --> número de ciclos a repetir al presionar start. (es decir, número de tubos a llenar). Máximo 200. Se puede aumentar en la línea 139 del programa.
5: Pausa --> tiempo de espera entre dos ciclos. Permite posicionar la manguera sobre un nuevo tubo vacío
6: Modo --> Tiene tres opciones. Dosifica (ejecuta un ciclo durante el que gira el número de veces seleccionado durante la calibración y luego frena. Hace una pausa y vuelve a arrancar hasta completar el número de ciclos indicados en el menú 4). Bombea (el motor gira a la velocidad seleccionada en menú 3) y Calibra (ejecuta un ciclo de calibración y luego vuelva automáticamente a modo Dosifica)
7: PGM --> permite almacenar dos volumenes diferentes para dispensar comida
8: Sentido --> sentido de giro
9:  M--> es el número de pasos que da el motor en el programa moscas (200 pasos es una vuelta). 
10: G--> es el número de pasos que da el motor en el programa gusanos (200 pasos es una vuelta). 
11: Guarda configuración en memoria: Al iniciar el programa carga valores por defecto en la memoria. Luego de calibrar (o modificar la velocidad, ciclos, pausa) se debe ejecutar esta opción del menú para que se guarden en la memoria. En caso contrario, la vez siguiente que se encienda el equipo volverá a los valores por defecto.
Inicio
Ajustar la posición del cubrerotor para que los rulemanes ajusten perfectamente bien la manguera, pero que no bloqueen el motor. La manguera está bien ajustada cuando al soplar por un extremo no es posible hacer circular aire.

Calibración
Seleccionar en el menú 7 un programa para calibrar (moscas o gusanos. O tubo grande/chico).
En menú 6 seleccionar la opción calibrar. Aparece un mensaje que dice: “Presionar hasta llenar el tubo”
Posicionar la manguera sobre un tubo vacío y mantener presionado el botón. El motor comenzará a girar lentamente. Soltar el botón cuando se alcance el nivel de comida deseado. El programa almacena el número de vueltas dado y calcula cuánto tiempo demorará en llenar el tubo (nuevo valor del menú 2) en función de la velocidad indicada en el menú 3. Luego muestra el  número de pasos y el número de vueltas del motor. Al finalizar la rutina pasa al modo dosifica automáticamente.

Dosificación
Girar la perilla hasta llegar al menú 1 y presionar el botón. Aparece la leyenda RUN. Según el modo, bombea, dosifica o calibra.
Si está en modo “bombea”, arranca el motor, cumple un ciclo y hace una pausa (de duración establecida en menú 5). Repite el proceso tantas veces como se haya indicado en el menú 4
La ejecución de la serie de ciclos se puede interrumpir presionando nuevamente la perilla. El efecto no es inmediato, primero tiene que terminar de ejecutar el ciclo actual y luego frena.



