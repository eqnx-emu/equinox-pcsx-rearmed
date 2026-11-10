#include "strings.hpp"

namespace {
struct Pair {
    const char* pt;
    const char* en;
};

// Same order as enum Str.
constexpr Pair kTable[] = {
    {"Continuar", "Continue"},
    {"Salvar estado", "Save state"},
    {"Carregar estado", "Load state"},
    {"Sair para o Equinox", "Exit to Equinox"},
    {"Estado salvo", "State saved"},
    {"Estado carregado", "State loaded"},
    {"N\xC3\xA3o foi poss\xC3\xADvel salvar", "Could not save"},
    {"Nenhum estado salvo", "No saved state"},
    {"Abra este emulador pelo Equinox", "Open this emulator from Equinox"},
    {"Disco não encontrado", "Disc not found"},
    {"Não foi possível ler o disco", "Could not read the disc"},
    {"Falta um arquivo do disco", "A disc file is missing"},
    {"Disco não suportado", "Unsupported disc"},
    {"Voltar ao Equinox", "Back to Equinox"},
    {"Disco %u de %u", "Disc %u of %u"},
    {"Vídeo", "Video"},
    {"Vídeo", "Video"},
    {"Tamanho", "Size"},
    {"Suavização", "Smoothing"},
    {"Filtro", "Filter"},
    {"Moldura", "Border"},
    {"Usar o padrão", "Use default"},
    {"Voltar", "Back"},
    {"Original", "Original"},
    {"8:7", "8:7"},
    {"4:3", "4:3"},
    {"Tela inteira", "Full screen"},
    {"Pixel", "Pixel"},
    {"Nítido", "Sharp"},
    {"Suave", "Soft"},
    {"Nenhum", "None"},
    {"Scanlines", "Scanlines"},
    {"NTSC", "NTSC"},
    {"Equinox", "Equinox"},
    {"Capa do jogo", "Game cover"},
    {"TV de tubo", "CRT TV"},
    {"Preto", "Black"},
    {"Não deu para guardar o ajuste", "Could not save the setting"},
    {"Disco trocado", "Disc changed"},
    {"Não deu para trocar o disco", "Could not change the disc"},
};
}  // namespace

const char* tr(Lang lang, Str s) {
    const auto i = static_cast<unsigned>(s);
    if (i >= sizeof kTable / sizeof kTable[0]) return "";
    return lang == Lang::PtBR ? kTable[i].pt : kTable[i].en;
}
