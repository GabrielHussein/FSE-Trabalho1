# FSE-Trabalho1

Trabalho para realizar monitoramento de salas de aula via um servidor central utilizando Raspberry Pi 4

Nome: Gabriel Alves Hussein
Matrícula: 17/0103200

## Instalação

Para instalar é necessário abrir dois terminais separados, um para o servidor central e ao menos um para um distribuído:

```bash
cd distribuido
make
```

```bash
cd central
make
```

## Execução

Para realizar execução é só ir na pasta bin e executar o bin gerado, para o servidor central é necessário apenas o bin, já nos distribuídos é necessário informar a sala que será utilizada na configuração, cada sala possui associações de pinos da WiringPI da RASP diferentes:

```bash
cd Central
bin/bin
```

```bash
cd Distributed
bin/bin sala1.txt
```

```bash
cd Distributed
bin/bin sala2.txt
```

## Modo de Uso

Para testes foram utilizados a RASP 43 com Uma instância do servidor central e duas instâncias de servidor distribuído, mesmo que não tenha funcionado corretamente foi possível conectar em paralelo essas duas instâncias e receber a resposta de conexão do servidor central.

Os acionamentos dos botões no dashboard refletem nos terminais principalmente do servidor distribuído onde houve mais evolução.
