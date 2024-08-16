# PiCLIs
Projeto Final da Disciplina PCS3732 - Laboratório de Processadores: PiCLIs

Este projeto foi pensado como uma adaptação e extensão do programa gdbstub apresentado em aula, mas fora do contexto de depuração. Ele atua como um CLI para o Raspberry Pi, permitindo controle sobre os diferentes módulos da placa, como a UART, as GPIOs (em específico o LED nativo) e a memória RAM.
É necessário usar este CLI com algum terminal serial de preferência, como o screen ou o Minicom, pois toda comunicação entre computador e placa é realizada pela UART.

Os comandos adicionados e adaptados estão nos seguintes formatos:

$pMORSE (mensagem) - Usa o LED verde da placa para sinalizar em código morse a mensagem fornecida.

$pBIN (número decimal) - Converte o número decimal positivo ou negativo para sua representação binária em complemento de 2.

$pECHO (mensagem) - Recebe a mensagem, e envia ela serialmente de volta ao remetente pela UART.

m (endereço inicial) (tamanho) - Adaptação do comando do gdbstub para leitura de memória. São mostrados no terminal (tamanho) bytes de dados a partir de (endereço inicial), sendo o endereço inicial dado em bytes.

M (endereço inicial) (tamanho) - Adaptação do comando do gdbstub para escrita de memória byte por byte. Deve ser seguido por (tamanho) caracteres hexadecimais, que reescrevem a memória na região determinada.

Para executar, apenas baixe todos os arquivos do repositório, execute o comando "make all", coloque os arquivos no cartão SD preparado para uso pelo Raspberry Pi 2 B (junto com os arquivos fixup.dat, .rtb, start.elf, config.txt, etc.), conecte um conversor USB-serial nos pinos correspondentes à interface UART e ligue o terminal serial de sua preferência.

Para usar os diferentes módulos da placa, usamos tanto instruções adaptadas do gdbstub, como as de manipulação de memória, quanto instruções originais personalizadas e específicas para propósitos distintos. Apresentaremos as instruções a seguir:
