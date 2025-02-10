# ✨ Tarefa06-Protocolo de Comunicação

<p align="center"> Repositório dedicado a Tarefa 06 do processo de capacitação do EmbarcaTech que envolve protocolos de comunicação na placa Raspberry Pi Pico W por meio da Plataforma BitDogLab.</p>

## :clipboard: Apresentação da tarefa

Para consolidar a compreensão dos conceitos relacionados ao uso de protocolos de comunicação no microcontrolador RP2040 e explorar as funcionalidades da placa de desenvolvimento BitDogLab, esta atividade propõe a implementação de um projeto prático envolvendo o envio de informações (caracteres) por meio do monitor serial que por fim são exibidas no display OLED ssd1306, como também, na matriz de led 5x5 ws2818b. Também é implementado interrupções acionadas por botões que controlam os níveis lógicos de LEDs.

## :dart: Objetivos

- Compreender o funcionamento e a aplicação de protocolos de comunicação em microcontroladores;

- Compreender o funcionamento e a aplicação de interrupções em microcontroladores;

- Implementar a técnica de debouncing via software para eliminar o efeito de bouncing em botões;

- Controlar e informar o estado lógico de LEDs por meio de botões

- Manipular e controlar LEDs comuns e LEDs endereçáveis WS2812;

- Toda informação que ocorre no projeto deve ser exibida no display OLED ssd1306;

- Modificação da biblioteca font.h (Fora adicionado letras minúsculas).

## :books: Descrição do Projeto

Utiizou-se a placa BitDogLab (que possui o microcontrolador RP2040) para a exibição no display OLED ssd1306 de informações enviadas por meio do monitor serial. Algumas informações enviadas também são mostradas na matriz de led 5x5 (números, especificamente). Os botões acionam interrupções que modificam os estados dos LEDs de forma externa ao *loop* principal.

## :walking: Integrantes do Projeto

- Matheus Pereira Alves

## :bookmark_tabs: Funcionamento do Projeto

- As informações foram repassadas pelo monitor serial com baud rate de 115200 (utilizou-se do Putty);
- É possível exibir no display as letras de a-z (minúsculas e maiúsculas) e os números;
- A matriz de LED (GPIO 7) exibe os números de 0-9 quando esses são enviados;
- O Botão A (GPIO 5) modifica o estado do LED verde por meio de uma interrupção, também envia uma exibição para o monitor serial e display OLED;
- O Botão B (GPIO 6) modifica o estado do LED azul por meio de uma interrupção, também envia uma exibição para o monitor serial e display OLED;
- Os botões possuem *Debounce* de 250 ms por meio de *software*;
- É utilizado I2C e UART para o cumprimento da tarefa.

## :camera: GIF mostrando o funcionamento do programa na placa BitDogLab
<p align="center">
  <img src=".github/display.gif" alt="GIF" width="345px" />
</p>

## :arrow_forward: Vídeo no youtube mostrando o funcionamento do programa na placa BitDogLab

<p align="center">
    <a href="https://www.youtube.com/watch?v=O4LO5Wcnx7Y">Clique aqui para acessar o vídeo</a>
</p>
