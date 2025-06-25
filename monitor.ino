#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int ldrPin = A0;
const int threshold = 150; // Limiar ajustado para detecção correta

// Variáveis para suavizar a leitura
int leituraAnterior = 0;
const int numLeituras = 5;
int leituras[numLeituras];
int indiceAtual = 0;
int total = 0;
int media = 0;

// Bitmaps para ícones (8x8 pixels)
const unsigned char PROGMEM iconeSol[] = {
  0x18, 0x24, 0x42, 0x99, 0x99, 0x42, 0x24, 0x18
};

const unsigned char PROGMEM iconeLua[] = {
  0x1C, 0x22, 0x40, 0x80, 0x80, 0x40, 0x22, 0x1C
};

void setup() {
  Serial.begin(9600);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha na alocação do SSD1306"));
    for(;;);
  }
  
  // Tela de inicialização
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("MONITOR");
  display.setCursor(25, 40);
  display.println("DE LUZ");
  display.display();
  delay(2000);
  
  pinMode(ldrPin, INPUT);
  
  // Inicializar array de leituras
  for (int i = 0; i < numLeituras; i++) {
    leituras[i] = 0;
  }
}

void loop() {
  // Fazer leitura suavizada
  int ldrValue = lerLDRSuavizado();
  
  Serial.print("Valor do LDR: ");
  Serial.print(ldrValue);
  Serial.print(" - Status: ");
  
  display.clearDisplay();
  
  // Cabeçalho
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(35, 0);
  display.println("MONITOR LUZ");
  
  // Linha separadora
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Valor numérico
  display.setCursor(0, 15);
  display.print("Valor: ");
  display.print(ldrValue);
  
  // Barra de progresso
  desenharBarraProgresso(ldrValue);
  
  // Status com ícone
  display.setTextSize(1);
  display.setCursor(35, 45);
  
  if(ldrValue > threshold) {  // Valor ALTO = Claro = Boa iluminação
    display.drawBitmap(10, 43, iconeSol, 8, 8, SSD1306_WHITE);
    display.println("ILUMINACAO RUIM");
    Serial.println("ILUMINACAO RUIM");
  } else {                    // Valor BAIXO = Escuro = Iluminação ruim
    display.drawBitmap(10, 43, iconeLua, 8, 8, SSD1306_WHITE);
    display.println("ILUMINACAO BOA");
    Serial.println("ILUMINACAO BOA");
  }
  
  // Indicador de atividade (ponto piscando)
  static bool pontoAtivo = false;
  pontoAtivo = !pontoAtivo;
  if (pontoAtivo) {
    display.fillCircle(120, 58, 2, SSD1306_WHITE);
  }
  
  display.display();
  delay(500);
}

// Função para fazer leitura suavizada (média móvel)
int lerLDRSuavizado() {
  // Subtrair a leitura mais antiga
  total = total - leituras[indiceAtual];
  
  // Fazer nova leitura
  leituras[indiceAtual] = analogRead(ldrPin);
  
  // Adicionar a nova leitura ao total
  total = total + leituras[indiceAtual];
  
  // Avançar para próxima posição
  indiceAtual = (indiceAtual + 1) % numLeituras;
  
  // Calcular média
  media = total / numLeituras;
  
  return media;
}

// Função para desenhar barra de progresso
void desenharBarraProgresso(int valor) {
  int larguraBarra = map(valor, 0, 1023, 0, 100);
  
  display.setCursor(0, 25);
  display.print("Intensidade:");
  
  // Contorno da barra
  display.drawRect(0, 35, 102, 6, SSD1306_WHITE);
  
  // Preenchimento da barra
  if (larguraBarra > 0) {
    display.fillRect(1, 36, larguraBarra, 4, SSD1306_WHITE);
  }
  
  // Percentual
  display.setCursor(105, 33);
  int percentual = map(valor, 0, 1023, 0, 100);
  display.print(percentual);
  display.print("%");
}

// Função para calibração automática (chame no setup se necessário)
void calibrarSensor() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(20, 20);
  display.println("CALIBRANDO...");
  display.setCursor(10, 35);
  display.println("Varie a iluminacao");
  display.display();
  
  int valorMax = 0;
  int valorMin = 1023;
  
  // Coletar dados por 10 segundos
  for (int i = 0; i < 100; i++) {
    int leitura = analogRead(ldrPin);
    if (leitura > valorMax) valorMax = leitura;
    if (leitura < valorMin) valorMin = leitura;
    
    // Mostrar progresso
    display.fillRect(10 + (i/10), 50, 2, 4, SSD1306_WHITE);
    display.display();
    
    delay(100);
  }
  
  // Calcular novo threshold
  int novoThreshold = (valorMax + valorMin) / 2;
  
  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("Novo limiar: ");
  display.println(novoThreshold);
  display.setCursor(10, 35);
  display.println("Calibracao concluida!");
  display.display();
  delay(3000);
}
