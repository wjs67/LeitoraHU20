# Leitor HU20 - Serial Number Reader

## Descrição
Este projeto lê o número serial da leitora biométrica digital **HU20** da Secugen conectada à USB.

## Requisitos
- **.NET 6.0** ou superior
- **Windows 10/11** (x64)
- **Leitor HU20 conectado à USB**
- **Driver do HU20 instalado** (incluído no SDK da Secugen)

## Compilação

```powershell
dotnet build -c Release
```

As DLLs nativas serão copiadas automaticamente para o diretório de saída.

## Execução

### Modo Release
```powershell
.\bin\Release\net6.0\LeitoraHU20.exe
```

### Modo Debug
```powershell
dotnet run
```

## Saída Esperada
Se o leitor estiver conectado e reconhecido, o programa imprime o número serial na saída padrão:
```
H54200804821
```

## Troubleshooting

### "Dispositivo HU20 não encontrado"
- Verifique se o leitor está conectado à USB
- Verifique se o LED do leitor está aceso
- Reinstale o driver: `C:\Program Files\SecuGen\Drivers\WBFU20`

### "Falha ao inicializar o SDK SecuGen"
- O SDK pode não estar instalado corretamente
- Execute o instalador do SecuGen FDx SDK Pro

### Warnings "ERR: Can't open the key"
- São avisos normais do SDK - não indicam erro
- O programa continua funcionando normalmente

## Estrutura do Projeto

```
Leitora HU20/
├── hu20_get_serial.cs                    # Código principal
├── LeitoraHU20.csproj                    # Configuração do projeto
├── sgfplib.dll                           # Biblioteca nativa principal
├── sgbledev.dll                          # Suporte Bluetooth
├── sgfdusdax64.dll                       # Suporte USB (x64)
├── sgfpamx.dll                           # Suporte de autenticação
├── sgwsqlib.dll                          # Suporte de qualidade
├── SecuGen.FDxSDKPro.DotNet.Windows.dll  # Wrapper .NET
└── bin/Release/net6.0/                   # Saída de compilação
```

## Dependências

- **SecuGen.FDxSDKPro.DotNet.Windows.dll** - SDK .NET para biometria
- **Drivers nativas** do SecuGen (copiadas automaticamente na compilação)

## Notas
- O projeto está compilado como x64 (PlatformTarget=x64)
- Todas as DLLs nativas são copiadas automaticamente durante a compilação
- O programa tenta múltiplos índices de dispositivo (0-9) para maior compatibilidade

## Autor
**Wellington Silva**
- Email: [swj67@protonmail.com](mailto:swj67@protonmail.com)
- GitHub: [github.com/wjs67](https://github.com/wjs67)

## Licença
Este projeto utiliza o SDK da Secugen. Verifique os termos de licença do SDK.
