#include <windows.h>
#include <tchar.h>

#include "resource.h"
#include "..\Global Data Structures\GameBoard.h"

#include "pipeFunctions.h"

/* ===================================================== */
/* Programa base (esqueleto) para aplicações Windows */
/* ===================================================== */
// Cria uma janela de nome "Janela Principal" e pinta fundo de branco
// Modelo para programas Windows:
// Composto por 2 funções:
// WinMain() = Ponto de entrada dos programas windows
// 1) Define, cria e mostra a janela
// 2) Loop de recepção de mensagens provenientes do Windows
// TrataEventos()= Processamentos da janela (pode ter outro nome)
// 1) É chamada pelo Windows (callback)
// 2) Executa código em função da mensagem recebida
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Connect(HWND, UINT, WPARAM, LPARAM);

// Nome da classe da janela (para programas de uma só janela, normalmente este nome é
// igual ao do próprio programa) "szprogName" é usado mais abaixo na definição das
// propriedades do objecto janela
TCHAR szProgName[] = TEXT("PipeWorks");

HINSTANCE hInstance;

TCHAR boardEventName[] = TEXT("BoardEvent");
TCHAR internalClientEvent[] = TEXT("ClientMoveEvent");
TCHAR moveEventName[] = TEXT("MoveEvent");
TCHAR pipeEventName[] = TEXT("PipeEvent");

// ============================================================================
// FUNÇÃO DE INÍCIO DO PROGRAMA: WinMain()
// ============================================================================
// Em Windows, o programa começa sempre a sua execução na função WinMain()que desempenha
// o papel da função main() do C em modo consola WINAPI indica o "tipo da função" (WINAPI
// para todas as declaradas nos headers do Windows e CALLBACK para as funções de
// processamento da janela)
// Parâmetros:
// hInst: Gerado pelo Windows, é o handle (número) da instância deste programa
// hPrevInst: Gerado pelo Windows, é sempre NULL para o NT (era usado no Windows 3.1)
// lpCmdLine: Gerado pelo Windows, é um ponteiro para uma string terminada por 0
// destinada a conter parâmetros para o programa
// nCmdShow: Parâmetro que especifica o modo de exibição da janela (usado em
// ShowWindow()
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd; // hWnd é o handler da janela, gerado mais abaixo por CreateWindow()
	MSG lpMsg; // MSG é uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp; // WNDCLASSEX é uma estrutura cujos membros servem para
	// definir as características da classe da janela
	
	hInstance = hInst;

		// ============================================================================
		// 1. Definição das características da janela "wcApp"
		// (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
		// ============================================================================
	wcApp.cbSize = sizeof(WNDCLASSEX); // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst; // Instância da janela actualmente exibida
	// ("hInst" é parâmetro de WinMain e vem
	// inicializada daí)
	wcApp.lpszClassName = szProgName; // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos; // Endereço da função de processamento da janela
	// ("TrataEventos" foi declarada no início e
	// encontra-se mais abaixo)
	wcApp.style = CS_HREDRAW | CS_VREDRAW; // Estilo da janela: Fazer o redraw se for
	// modificada horizontal ou verticalmente
	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION); // "hIcon" = handler do ícon normal
	// "NULL" = Icon definido no Windows
	// "IDI_AP..." Ícone "aplicação"
	wcApp.hIconSm = LoadIcon(NULL, IDI_ERROR); // "hIconSm" = handler do ícon pequeno
	// "NULL" = Icon definido no Windows
	// "IDI_INF..." Ícon de informação
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW); // "hCursor" = handler do cursor (rato)
	// "NULL" = Forma definida no Windows
	// "IDC_ARROW" Aspecto "seta"
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDC_CLIENT); // Classe do menu que a janela pode ter
	// (NULL = não tem menu)
	wcApp.cbClsExtra = 0; // Livre, para uso particular
	wcApp.cbWndExtra = 0; // Livre, para uso particular
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	// "hbrBackground" = handler para "brush" de pintura do fundo da janela. Devolvido por
	// "GetStockObject".Neste caso o fundo será branco

	// ============================================================================
	// 2. Registar a classe "wcApp" no Windows
	// ============================================================================
	if (!RegisterClassEx(&wcApp)) return(0);

	// ============================================================================
	// 3. Criar a janela
	// ============================================================================
	hWnd = CreateWindow(szProgName, // Nome da janela (programa) definido acima
		TEXT("PipeWorks"), // Texto que figura na barra do título
		WS_OVERLAPPEDWINDOW, // Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT, // Posição x pixels (default=à direita da última)
		CW_USEDEFAULT, // Posição y pixels (default=abaixo da última)
		(32 * 20) + (64 * 3), // Largura da janela (em pixels)
		(32 * 20) + (64 * 2), // Altura da janela (em pixels)
		(HWND)HWND_DESKTOP, // handle da janela pai (se se criar uma a partir de
		// outra) ou HWND_DESKTOP se a janela for a primeira,
		// criada a partir do "desktop"
		(HMENU)NULL, // handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst, // handle da instância do programa actual ("hInst" é
		// passado num dos parâmetros de WinMain()
		0); // Não há parâmetros adicionais para a janela

// ============================================================================
// 4. Mostrar a janela
// ============================================================================
	ShowWindow(hWnd, nCmdShow); // "hWnd"= handler da janela, devolvido por
	// "CreateWindow"; "nCmdShow"= modo de exibição (p.e.
	// normal/modal); é passado como parâmetro de WinMain()

	UpdateWindow(hWnd); // Refrescar a janela (Windows envia à janela uma
	// mensagem para pintar, mostrar dados, (refrescar)...

	// ============================================================================
	// 5. Loop de Mensagens
	// ============================================================================
	// O Windows envia mensagens às janelas (programas). Estas mensagens ficam numa fila de
	// espera até que GetMessage(...) possa ler "a mensagem seguinte"
	// Parâmetros de "getMessage":
	// 1)"&lpMsg"=Endereço de uma estrutura do tipo MSG ("MSG lpMsg" ja foi declarada no
	// início de WinMain()):
	// HWND hwnd handler da janela a que se destina a mensagem
	// UINT message Identificador da mensagem
	// WPARAM wParam Parâmetro, p.e. código da tecla premida
	// LPARAM lParam Parâmetro, p.e. se ALT também estava premida
	// DWORD time Hora a que a mensagem foi enviada pelo Windows
	// POINT pt Localização do mouse (x, y)
	// 2)handle da window para a qual se pretendem receber mensagens (=NULL se se pretendem
	// receber as mensagens para todas as janelas pertencentes à thread actual)
	// 3)Código limite inferior das mensagens que se pretendem receber
	// 4)Código limite superior das mensagens que se pretendem receber
	// NOTA: GetMessage() devolve 0 quando for recebida a mensagem de fecho da janela,
	// terminando então o loop de recepção de mensagens, e o programa
	while (GetMessage(&lpMsg, NULL, 0, 0))
	{
		TranslateMessage(&lpMsg); // Pré-processamento da mensagem (p.e. obter código
		// ASCII da tecla premida)
		DispatchMessage(&lpMsg); // Enviar a mensagem traduzida de volta ao Windows, que
		// aguarda até que a possa reenviar à função de
		// tratamento da janela, CALLBACK TrataEventos (abaixo)
	}
	// ============================================================================
	// 6. Fim do programa
	// ============================================================================
	return((int)lpMsg.wParam); // Retorna sempre o parâmetro wParam da estrutura lpMsg
}
// ============================================================================
// FUNÇÃO DE PROCESSAMENTO DA JANELA
// Esta função pode ter um nome qualquer: Apenas é necesário que na inicialização da
// estrutura "wcApp", feita no início de WinMain(), se identifique essa função. Neste
// caso "wcApp.lpfnWndProc = WndProc"
//
// WndProc recebe as mensagens enviadas pelo Windows (depois de lidas e pré-processadas
// no loop "while" da função WinMain()
// Parâmetros:
// hWnd O handler da janela, obtido no CreateWindow()
// messg Ponteiro para a estrutura mensagem (ver estrutura em 5. Loop...
// wParam O parâmetro wParam da estrutura messg (a mensagem)
// lParam O parâmetro lParam desta mesma estrutura
//
// NOTA:Estes parâmetros estão aqui acessíveis o que simplifica o acesso aos seus valores

// A função EndProc existe um "switch..." com "cases" que descriminam a mensagem
// recebida e a tratar. Estas mensagens são identificadas por constantes (p.e.
// WM_DESTROY, WM_CHAR, WM_KEYDOWN, WM_PAINT...) definidas em windows.h
// ============================================================================
LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	HDC dc;
	PAINTSTRUCT ps;
	int x, y;

	static HBITMAP hBitmap_CellEmpty = NULL;
	static HBITMAP hBitmap_CellHPipe = NULL;
	static HBITMAP hBitmap_CellVPipe = NULL;
	static HBITMAP hBitmap_CellULPipe = NULL;
	static HBITMAP hBitmap_CellURPipe = NULL;
	static HBITMAP hBitmap_CellDLPipe = NULL;
	static HBITMAP hBitmap_CellDRPipe = NULL;

	static PieceType g_currPiece;
	static Data* data = NULL;

	switch (messg)
	{
	case WM_PAINT:
		dc = BeginPaint(hWnd, &ps);
		HDC auxdc;
		if (data->fc->gameboard.board != NULL)
		{
			GameBoard* board = &data->fc->gameboard;
			//if (board->x < 0 || board->x > 20) break;
			for (int y = 0; y < board->y; y++)
			{
				for (int x = 0; x < board->x; x++)
				{
					auxdc = CreateCompatibleDC(dc);

					if (board->board[y][x].piece == E)
						SelectObject(auxdc, (HGDIOBJ)hBitmap_CellEmpty);
					else if (board->board[y][x].isWall == TRUE)
						SelectObject(auxdc, (HGDIOBJ)hBitmap_CellEmpty);
					else if (board->board[y][x].piece == H)
						SelectObject(auxdc, (HGDIOBJ)hBitmap_CellHPipe);
					else if (board->board[y][x].piece == V)
						SelectObject(auxdc, (HGDIOBJ)hBitmap_CellVPipe);
					else if (board->board[y][x].piece == DL)
						SelectObject(auxdc, (HGDIOBJ)hBitmap_CellDLPipe);
					else if (board->board[y][x].piece == DR)
						SelectObject(auxdc, (HGDIOBJ)hBitmap_CellDRPipe);
					else if (board->board[y][x].piece == UL)
						SelectObject(auxdc, (HGDIOBJ)hBitmap_CellULPipe);
					else if (board->board[y][x].piece == UR)
						SelectObject(auxdc, (HGDIOBJ)hBitmap_CellURPipe);

					BitBlt(dc, x * 32, y * 32, 32, 32, auxdc, 0, 0, SRCCOPY);
					DeleteDC(auxdc);
				}
			}
		}
		
		//Next Piece
		auxdc = CreateCompatibleDC(dc);
		if (g_currPiece == H)
			SelectObject(auxdc, (HGDIOBJ) hBitmap_CellHPipe);
		else if (g_currPiece == V)
			SelectObject(auxdc, (HGDIOBJ) hBitmap_CellVPipe);
		else if (g_currPiece == DL)
			SelectObject(auxdc, (HGDIOBJ) hBitmap_CellDLPipe);
		else if (g_currPiece == DR)
			SelectObject(auxdc, (HGDIOBJ) hBitmap_CellDRPipe);
		else if (g_currPiece == UL)
			SelectObject(auxdc, (HGDIOBJ) hBitmap_CellULPipe);
		else if (g_currPiece == UR)
			SelectObject(auxdc, (HGDIOBJ) hBitmap_CellURPipe);
		else
			SelectObject(auxdc, (HGDIOBJ) hBitmap_CellEmpty);

		BitBlt(dc, (32 * 20) + (32 * 2), 0, 32, 32, auxdc, 0, 0, SRCCOPY);
		DeleteDC(auxdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_CONNECT:
			CreateDialog(hInstance, MAKEINTRESOURCE(IDD_IPBOX), hWnd, Connect);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, messg, wParam, lParam);
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		x = LOWORD(lParam);
		y = HIWORD(lParam);

		int conX = (x / 32);
		int conY = (y / 32);

		bc.x = conX;
		bc.y = conY;
		bc.piece = g_currPiece;
		SetEvent(data->hMoveEvent);
	}
		break;
	case WM_RBUTTONDOWN:
		if (g_currPiece == H)
			g_currPiece = V;
		else if (g_currPiece == V)
			g_currPiece = H;
		else if (g_currPiece == DL)
			g_currPiece = UL;
		else if (g_currPiece == UL)
			g_currPiece = UR;
		else if (g_currPiece == UR)
			g_currPiece = DR;
		else if (g_currPiece == DR)
			g_currPiece = DL;

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_MOUSEWHEEL:
		if (g_currPiece == H)
			g_currPiece = V;
		else if (g_currPiece == V)
			g_currPiece = DL;
		else if (g_currPiece == DL || g_currPiece == UL || g_currPiece == UR || g_currPiece == DR)
			g_currPiece = H;

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_DESTROY: // Destruir a janela e terminar o programa
		PostQuitMessage(0);
		break;
	case WM_CREATE:
	{
		hBitmap_CellEmpty = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));
		hBitmap_CellVPipe = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP2));
		hBitmap_CellHPipe = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP3));
		hBitmap_CellDLPipe = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP4));
		hBitmap_CellDRPipe = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP5));
		hBitmap_CellULPipe = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP6));
		hBitmap_CellURPipe = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP7));
		g_currPiece = H;

		data = initData();

		PipeThreadInfo* pti = malloc(sizeof(PipeThreadInfo));
		if (pti == NULL) return -1;
		pti->data = data;
		pti->hWnd = hWnd;
		if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pipeThread, (LPVOID)pti, 0, NULL) == NULL) return -1;
	}
		break;
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// não é efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break; // break tecnicamente desnecessário por causa do return
	}

	return(0);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for connect box.
INT_PTR CALLBACK Connect(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_IPCANCEL || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDC_IPCONNECT)
		{
			//Replace with server connect
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}