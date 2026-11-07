# PoloOS

O sistema usado pelo Marco Polo, o robô de Trekking da [Robota](https://www.instagram.com/robota.ufsc/), que estreou na
[RSM Challenge de 2025](https://events.robocore.net/rsm-2025/) com o codinome Cedos.

## Equipamento

- ESP32
- Tira de colorida de LEDs individualmente endereçáveis
- _Pack_ de baterias

## Esquema de Ligações

## Compilação

Antes de compilar é necessário configurar o Bluetooth para funcionar no modo clássico, para isso basta abrir o menuconfig, selecionar as opções "Component config" > "Bluetooth", e mudar as seguintes opções:

- "Bluetooth" > "Host" para "Bluetooth - Dual-mode"
- "Controller Options" > "Bluetooth controller mode" para "BR/EDR Only"
