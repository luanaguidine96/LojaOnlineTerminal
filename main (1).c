
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h> /* necessario para falar com a porta serial (Arduino) no Windows */

/* Taxas de cambio fixas */
#define TAXA_DOLAR 5.20
#define TAXA_EURO 5.65
#define TAXA_LIBRA 6.60
#define TAXA_YEN 0.035
#define TAXA_BTC 320000.0

#define NUM_FRUTAS 4 /* Quantidade de frutas no catalogo */

/* Limite do "semaforo" de estoque:
   estoque == 0 -> VERMELHO (sem estoque)
   0 < estoque <= LIMITE -> AMARELO (estoque baixo)
   estoque > LIMITE -> VERDE (estoque ok) */
#define LIMITE_BAIXO 10

   /* -------------------------------------------------------------
   CONFIGURACAO DA PORTA SERIAL DO ARDUINO
   Troque "COM3" pela porta que aparece no Gerenciador de
   Dispositivos do Windows quando o Arduino esta conectado.
   ------------------------------------------------------------- */
#define PORTA_SERIAL "COM4"

   /* Estrutura que cria uma fruta no estoque
   é como criar um "molde" para guardar dados juntos */
struct Fruta {
 char nome[50];
 float preco;
 int estoque_un;
};
/*em vez de ter 4 variáveis separadas
agrupa tudo em uma caixinha chamada Fruta*/

/* Variaveis*/
float saldo = 1000.00; /* Saldo inicial do usuario - fixo */
float carrinho_total = 0.00; /* Acumula o total das compras */

/* Serial / Arduino */
HANDLE hSerial;
int serial_ok = 0; /* 1 se conseguiu abrir a porta com o Arduino, 0 caso contrario */

/* Padrões - chamar a tela tal -função executa alguma coisa
mas não devolve nenhum valor*/

void cabecalho(void);
void menu_principal(struct Fruta frutas[]);/*recebe um vetor de frutas*/
void tela_loja(struct Fruta frutas[]);/*exibe todas as frutas disponíveis para compra*/
/*
imprime as frutas
pergunta qual comprar
chama comprar_fruta*/
void tela_caixa(struct Fruta frutas[]);/*informações do caixa*/
void tela_cambio(struct Fruta frutas[]);/*tela de conversões de moedas*/
void tela_extrato(struct Fruta frutas[]);/*exibe um histórico de compras*/

void comprar_fruta(struct Fruta frutas[], int idx);/*indica qual fruta será comprada*/

void abrir_serial(void); /* abre a conexao com o Arduino */
void atualizar_leds(struct Fruta frutas[]); /* envia o estado do estoque para o Arduino acender o "semaforo" de LEDs */

/*fim das declarações do void*/

/*LUANA*/
int main(void) {
 struct Fruta frutas[NUM_FRUTAS] = {
  {"* Brigadeiro tradicional ", 2.50f, 20},
  {"* Brownie ", 3.00f, 5},
  {"* Trufa ", 5.00f, 30},
  {"* Pipoca de sal", 5.00f, 8},
 
 };//preenchendo os valores da caixinha

 abrir_serial(); /* tenta conectar com o Arduino */
 atualizar_leds(frutas); /* manda o estado inicial do estoque para os LEDs */

 menu_principal(frutas);

 if (serial_ok) CloseHandle(hSerial);
 return 0;
}

/*Arduino e serial usb*/

void abrir_serial(void) {

    hSerial = CreateFileA(PORTA_SERIAL,
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING,
        0, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Erro ao conectar com Arduino.\n");
        serial_ok = 0;
        return;//exception
    }

    DCB dcb = {0};

    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = 9600;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;//comunicaçao em byte mesma velocidade

    SetCommState(hSerial, &dcb);

    Sleep(2000);//pausinha

    serial_ok = 1;

    printf("Arduino conectado!\n");
}


void atualizar_leds(struct Fruta frutas[]) {

    unsigned char estados[NUM_FRUTAS];

    for (int i = 0; i < NUM_FRUTAS; i++) {

        if (frutas[i].estoque_un == 0)
            estados[i] = 0;

        else if (frutas[i].estoque_un <= LIMITE_BAIXO)
            estados[i] = 1;

        else
            estados[i] = 2;
    }

    DWORD bytes;

    WriteFile(hSerial, estados, NUM_FRUTAS, &bytes, NULL);
}


/*LUANA*/


/* void cabecalho(void)*/
void cabecalho(void) {
 printf("\n ----------------------------------------------------------------------\n");
 printf("| 201 MERCADINHO |\n");
 printf("| Doces Saborosos & Caixa Eletronico |\n");
 printf("\n ----------------------------------------------------------------------\n"); printf("\n");
 printf(" Saldo atual: R$ %.2f\n", saldo);
 printf(" Carrinho: R$ %.2f\n", carrinho_total);
 printf("\n");
 printf("-------------------------------------------------------------------------\n");
}


/*void menu_principal(struct Fruta frutas[]);*/
void menu_principal(struct Fruta frutas[]) {
 int op; //cria a opçao

 while (1) {
  cabecalho();
  //chamar as opçoes
  printf("\n MENU PRINCIPAL\n\n");
  printf(" 1. Loja \n");
  printf(" 2. Caixa Eletronico\n");
  printf(" 3. Conversor de Moedas\n");
  printf(" 4. Pagar carrinho\n");
  printf(" 0. Sair\n\n");
  printf(" - > Opcao: ");
  scanf("%d", &op); // le a opçao digitada

  switch (op) { //se opçao for:
  case 1:
   tela_loja(frutas);
   break;//chamar os void
  case 2:
   tela_caixa(frutas);
   break;
  case 3:
   tela_cambio(frutas);
   break;
  case 4:
   tela_extrato(frutas);
   break;
  case 0:
   printf("\n Ate logo!\n\n");
   if (serial_ok) CloseHandle(hSerial);
   exit(0); /* CORRIGIDO: antes caia no default e nao encerrava o programa */
  default:
   printf(" Opcao invalida!\n");
  }
 }
}




/* void tela_loja(struct Fruta frutas[])*/
void tela_loja(struct Fruta frutas[]) {

 int op;//cria opçap

 while (1) {

  cabecalho();
  printf("\n LOJA DE DOCES (preco por unidade)\n\n");
 printf("| %-3s | %-25s | %-9s | %-8s |\n",
    "Num", "Doces", "R$/un", "Estoque");
  printf("\n");// string ocupa x carcateres,serve para ajudar alinhar esteticamente na tabelinha


  //percorre todas as frutas do vetor frutas enquanto i for menor que 8
  int i;
  for (i = 0; i < NUM_FRUTAS; i++)

 { // para conferir e passar por todas as frutas
    if (frutas[i].estoque_un == 0) // se estiver sem estoque
        printf("| %-3d | %-25s | R$ %6.2f | %3d un | [SEM ESTOQUE]\n",
            i + 1, frutas[i].nome, frutas[i].preco, frutas[i].estoque_un);
    else // se tiver estoque
        printf("| %-3d | %-25s | R$ %6.2f | %3d un |\n",
            i + 1, frutas[i].nome, frutas[i].preco, frutas[i].estoque_un);
}



  printf("\n Digite o numero do doce para comprar\n");
  printf(" 0 = Sair, 100 = Menu\n\n");
  printf(" - > Opcao: ");
  scanf("%d", &op);

  if (op == 0) break;
  if (op == 100) {
   menu_principal(frutas);
   return;
  }
  // se for um numero menor que os disponiveis ou maior vai dar erro
  if (op >= 1 && op <= NUM_FRUTAS)
   comprar_fruta(frutas, op - 1);
  else {
   printf(" Opcao invalida!\n");

  }
 }
}
/*vodigo 3 FIM */

void comprar_fruta(struct Fruta frutas[], int idx) */
void comprar_fruta(struct Fruta frutas[], int idx) {
 //idx é uma variavel tipo o i no contador

 int un; // Guarda quantas unidades a pessoa quer comprar.
 int conf; // Guarda se a pessoa confirmou a compra ou não.

 cabecalho(); // Mostra o cabeçalho do programa.

 // Mostra qual fruta foi escolhida com base no idx selecionado , ou id
 printf("\n Comprando: %s\n", frutas[idx].nome);

 // Mostra o preço de 1 unidade dessa fruta.
 printf(" Preco: R$ %.2f/un\n", frutas[idx].preco);

 // Mostra quanto ainda tem dessa fruta no estoque.
 printf(" Estoque: %d un\n\n", frutas[idx].estoque_un);

 // Pergunta quantos a pessoa quer comprar.
 printf(" Quantas unidades deseja? ");

 // Pega o valor digitado.
 scanf("%d", &un);

 // Se a pessoa digitar 0 ou um número menor, não deixa continuar.
 if (un <= 0) {
  printf(" Quantidade invalida.\n");

  // Sai da função porque esse valor não serve.
  return;
 }

 // Se pedir mais do que tem no estoque, também não deixa.
 if (un > frutas[idx].estoque_un) {
  printf(" Sem estoque suficiente!\n");

  // Sai da função.
  return;
 }

 // Faz a conta do valor total da compra.
 float total = un * frutas[idx].preco;

 // Mostra quanto vai ficar a compra.
 printf("\n %d un de %s = R$ %.2f\n", un, frutas[idx].nome, total);

 // Pergunta se quer confirmar a compra.
 printf(" Confirmar? (1=Sim / 0=Nao): ");

 // Pega a resposta da pessoa.
 scanf("%d", &conf);

 // Se digitou 1, confirma a compra.
 if (conf == 1) {

  // Soma esse valor no total do carrinho.
  carrinho_total += total;

  // Tira do estoque a quantidade comprada.
  frutas[idx].estoque_un -= un;

  // Avisa que deu tudo certo.
  printf(" Adicionado ao carrinho!\n");

  // CORRIGIDO: estoque mudou, entao avisa o Arduino para
  // atualizar o semaforo de LEDs com o novo estado do estoque.
  atualizar_leds(frutas);

 }
 else {

  // Se não confirmou, cancela a compra.
  printf(" Compra cancelada.\n");
 }

}
//fim codigo 4

//CODIGO 5

void tela_caixa(struct Fruta frutas[]) {
 int op;

 while (1) {
  cabecalho();
  printf("\n CAIXA ELETRONICO\n\n");
  printf(" 1. Sacar\n");
  printf(" 2. Depositar\n");
  printf(" 100. Menu\n");
  printf(" 0. Sair\n\n");
  printf(" - > Opcao: ");
  scanf("%d", &op);
  // Essa parte é responsavel por ser a exibição das escolhas(Ex: Opção 1- Sacar).//
  if (op == 0) break;
  if (op == 100) {
   menu_principal(frutas);
   return;
  }
  float valor;

  if (op == 1) {
   printf("\n $ Valor a sacar: R$ ");
   scanf("%f", &valor);
   // Essa parte é responsavel por exibir o valor que irá ser sacado.//
   if (valor <= 0) {
    printf(" Valor invalido.\n");
   }
   else if (valor > saldo) {
    printf(" Saldo insuficiente! Saldo: R$ %.2f\n", saldo);
   }
   else {
    saldo -= valor; // CORRIGIDO: sacar tem que TIRAR do saldo (estava somando)
    printf(" Saque de R$ %.2f realizado!\n", valor);
    printf(" Novo saldo: R$ %.2f\n", saldo);
    // Essa etapa é responsável pelo cáuculo e exibição do seu saldo, pois se seu saldo for menor que 0(zero),o saque não é realizado.//

   }


  }
  else if (op == 2) {
   printf("\n Valor a depositar: R$ ");
   scanf("%f", &valor);
   if (valor <= 0) {
    printf(" Valor invalido.\n");
   }
   else {
    saldo += valor; // CORRIGIDO: depositar tem que SOMAR ao saldo (estava subtraindo)
    printf(" Deposito de R$ %.2f realizado!\n", valor);
    printf(" Novo saldo: R$ %.2f\n", saldo);
    // Essa etapa demonstra o cáuculo e exibe o modo de depositar em nosso mercadinho.//
   }


  }
 }
}


//CODIGO 6
/* void tela_cambio(void)*/
void tela_cambio(struct Fruta frutas[]) {
 int op;

 while (1) {

  cabecalho();
  printf("\n CONVERSOR DE MOEDAS\n\n");
  printf(" Dolar = R$ %.2f\n", TAXA_DOLAR);
  printf(" Euro = R$ %.2f\n", TAXA_EURO);
  printf(" Libra = R$ %.2f\n", TAXA_LIBRA);
  printf(" Iene = R$ %.3f\n", TAXA_YEN);
  printf(" Bitcoin= R$ %.2f\n\n", TAXA_BTC);
  printf(" 1. Reais -> Dolar\n");
  printf(" 2. Reais -> Euro\n");
  printf(" 3. Reais -> Libra\n");
  printf(" 4. Reais -> Iene\n");
  printf(" 5. Reais -> Bitcoin\n");
  printf(" 6. Dolar -> Reais\n");
  printf(" 7. Euro -> Reais\n");
  printf(" 8. Libra -> Reais\n");
  printf(" 100. Menu \n");
  printf(" 0. Sair\n\n");
  printf(" - > Opcao: ");
  scanf("%d", &op);
  if (op == 0) break;
  if (op == 100) {
   menu_principal(frutas);
   return;
  }
  float valor, resultado;
  printf(" Valor: ");
  scanf("%f", &valor);

  switch (op) {
  case 1:
   resultado = valor / TAXA_DOLAR;
   printf(" R$ %.2f = $ %.2f USD\n", valor, resultado);
   break;
  case 2:
   resultado = valor / TAXA_EURO;
   printf(" R$ %.2f = EUR %.2f\n", valor, resultado);
   break;
  case 3:
   resultado = valor / TAXA_LIBRA;
   printf(" R$ %.2f = GBP %.2f\n", valor, resultado);
   break;
  case 4:
   resultado = valor / TAXA_YEN;
   printf(" R$ %.2f = JPY %.0f\n", valor, resultado);
   break;
  case 5:
   resultado = valor / TAXA_BTC;
   printf(" R$ %.2f = BTC %.8f\n", valor, resultado);
   break;
  case 6:
   resultado = valor * TAXA_DOLAR;
   printf(" $ %.2f USD = R$ %.2f\n", valor, resultado);
   break;
  case 7:
   resultado = valor * TAXA_EURO;
   printf(" EUR %.2f = R$ %.2f\n", valor, resultado);
   break;
  case 8:
   resultado = valor * TAXA_LIBRA;
   printf(" GBP %.2f = R$ %.2f\n", valor, resultado);
   break;
  default:
   printf(" Opcao invalida.\n");
  }

 }
}


/* void tela_extrato(void) */
/* Função que exibe o extrato da conta e permite pagar o carrinho */
void tela_extrato(struct Fruta frutas[]) {
 int conf; // Armazena a confirmação do usuário para realizar o pagamento

 // Exibe o cabeçalho do programa
 cabecalho();

 // Mostra o título da seção
 printf("\n EXTRATO\n\n");

 // mostra o saldo atual da conta
 printf(" Saldo na conta: R$ %8.2f\n", saldo);

 //mostra o valor total dos produtos no carrinho
 printf(" Total carrinho: R$ %8.2f\n", carrinho_total);

 // Linha decorativa
 printf("--------------------------------------------\n");

 // Calcula quanto vai ficar de saldo após o pagamento, e ja tarnsforma numa variavel
 float apos_pagar = saldo - carrinho_total;

 // Verifica se há saldo suficiente para pagar o carrinho, se o saldo saldo apos pagar for maior que 0
 //for positivo vai aparecer normal, se nao vai ter o tracinho negativo
 if (apos_pagar >= 0)
  printf(" Saldo apos pagar: R$ %8.2f\n", apos_pagar);
 else
  printf(" Saldo insuficiente! Faltam R$ %.2f\n", -apos_pagar);

 // Só oferece a opção de pagamento se houver itens no carrinho
 if (carrinho_total > 0) {
  printf("\n Deseja pagar o carrinho? (1=Sim / 0=Nao): ");
  printf("\n=========================================\n");
  printf(" PAGAMENTO VIA PIX \n");
  printf("=========================================\n");
  printf(" Recebedor : Luana Bosco Guidine \n");
  printf(" Banco : Banco do Bradesco \n");
  printf(" Chave PIX : 31 994182757 \n");
  printf(" Valor : R$ %.2f\n", carrinho_total);
  printf("-----------------------------------------\n");
  printf(" Abra o aplicativo do seu banco.\n");
  printf(" Escolha a opcao 'PIX'.\n");
  printf(" Copie a chave acima e realize o pagamento.\n");
  printf("-----------------------------------------\n");
  printf(" Apos concluir o pagamento,\n");
  printf(" confirme abaixo digitando 1\n");
  printf("=========================================\n");
  scanf("%d", &conf);

  // Verifica se o usuário confirmou o pagamento
  // CORRIGIDO: as chaves estavam erradas e faziam o "else" (saldo
  // insuficiente) responder ao "conf != 1" em vez de responder ao
  // "saldo < carrinho_total". Alem disso um "return" fora de lugar
  // fazia a mensagem de saldo insuficiente nunca aparecer.
  if (conf == 1) {

   // Confirma se há saldo suficiente
   if (saldo >= carrinho_total) {
    // Desconta o valor do carrinho do saldo
    saldo -= carrinho_total;

    // Esvazia o carrinho após o pagamento
    carrinho_total = 0.0f;

    // Informa que o pagamento foi realizado
    printf("\n Pagamento efetuado!\n");
   }
   else {
    // Informa que o saldo é insuficiente
    printf("\n Saldo insuficiente.\n");
   }
  }
 }
}
