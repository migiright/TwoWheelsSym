#include "DxLib.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include "MyMath.hpp"

using namespace std;

DINPUT_JOYSTATE joyState;

int joypadInputState; //押されているボタン
int joypadPushedState; //押されていない状態から押されている状態になったボタン
constexpr double Fps = 60; //1秒あたりのフレーム数
constexpr double Cycle = 1.0 / 60000; //微小時間
double c1 = 200, c2 = 120; //q[0]とq[1]の制約
bool virtualEnabled = true; //コントローラーの入力を仮想入力に入れるか直接実際の入力に入れるか
Vector<3> q; //元のシステムの状態
Vector<3> xi2; //

//元のシステムのg(q)
Matrix<3, 2> gq(const Vector<3> &q)
{
	Matrix<3, 2> ret;
	ret(0, 0) = cos(q[2]);
	ret(0, 1) = 0;
	ret(1, 0) = sin(q[2]);
	ret(1, 1) = 0;
	ret(2, 0) = 0;
	ret(2, 1) = 1;
	return ret;
}

//チェインドシステムの状態に変換
Vector<3> convertToChained(const Vector<3> q)
{
	return Vector<3>{q[0], tan(q[2]), q[1]};
}

//チェインドシステムの状態をシステム蘇生変換後のチェインドシステムの状態に変換
Vector<3> phi(const Vector<3> x)
{
	Vector<3> xi;
	xi[0] = tan(M_PI * x[0] / (2 * c1));
	xi[2] = tan(M_PI * x[2] / (2 * c2));
	xi[1] = c1 * (xi[2] * xi[2] + 1) * x[1] / (c2 * (xi[0] * xi[0] + 1));
	return xi;
}

//チェインドシステムの状態を元の状態に変換
Vector<3> convertFeomChained(const Vector<3> xi)
{
	Vector<3> th;
	th[0] = xi[0]; //システム蘇生変換後のチェインドシステムからシステム蘇生変換後のもとのシステムへ
	th[1] = xi[2];
	th[2] = atan(xi[1]) + (M_PI / 2 < q[2] && q[2] < 3 * M_PI / 2 ? M_PI : 0);
	return th;
}

void resetSystem()
{
	q[0] = q[1] = q[2] = 0;
	xi2[0] = xi2[1] = xi2[2] = 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 画面モードのセット
	SetGraphMode(640, 480, 16);
	ChangeWindowMode(TRUE);
	if(DxLib_Init() == -1) return -1;
	// 描画先画面を裏画面にセット
	SetDrawScreen(DX_SCREEN_BACK);

	Vector<3> v{1.0, 2.0, 3.0};

	// ループ
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
		clsDx(); //画面消去

		// コントローラ入力取得
		GetJoypadDirectInputState(DX_INPUT_PAD1, &joyState);
		int pjs = joypadInputState;
		joypadInputState = GetJoypadInputState(DX_INPUT_PAD1);
		//前フレームの入力(pjs)が0、かつ今回の入力(joypadInputState)が1のとき、1
		joypadPushedState = ~pjs & joypadInputState;

		//Aボタンで仮想入力と実際の入力の切り替え
		if(joypadPushedState & PAD_INPUT_1) virtualEnabled = !virtualEnabled;
		//Xボタンでリセット
		if(joypadPushedState & PAD_INPUT_3) resetSystem();

		Vector<3> th; //システム蘇生変換後のもとのシステム
		Vector<2> nu, mu;
		for(int i = 0; i < 1 / Cycle / Fps; ++i) { //1フレームで1/60
			//+入力を決める
			//状態変数を変換
			Vector<3> x(convertToChained(q)); //元のシステムからチェインドシステムへ
			Vector<3> xi(phi(x));
			th[0] = xi[0]; //システム蘇生変換後のチェインドシステムからシステム蘇生変換後のもとのシステムへ
			th[1] = xi[2];
			th[2] = atan(xi[1]) + (M_PI / 2 < q[2] && q[2] < 3 * M_PI / 2 ? M_PI : 0);
			//入力をpadから求めて入力変換
			if(virtualEnabled) {
				nu[0] = static_cast<double>(joyState.Y) * -0.005; //システム蘇生変換後の元のシステムの仮想入力
				nu[1] = static_cast<double>(joyState.Rx) * 0.004;
				Vector<2> v{ //システム蘇生変換後の元のシステムの仮想入力からシステム蘇生変換後のチェインドシステムの仮想入力へ
					nu[0] * cos(th[2]),
					nu[1] / (cos(th[2]) * cos(th[2])),
				};
				Vector<2> u{//システム蘇生変換後のチェインドシステムの仮想入力からチェインドシステムの入力へ
					2 * c1 * v[0] / (M_PI * (1 + xi[0] * xi[0])),
					c2 * (2 * xi[1] * v[0] * (xi[0] - xi[1] * xi[2] * (xi[0] * xi[0] + 1)) + v[1] * (xi[0] * xi[0] + 1)) / (c1 * (xi[2] * xi[2] + 1)),
				};
				mu[0] = u[0] / cos(q[2]);
				mu[1] = u[1] * cos(q[2]) * cos(q[2]);
			} else {
				mu[0] = static_cast<double>(joyState.Y) * -0.3; //元のシステムの入力
				mu[1] = static_cast<double>(joyState.Rx) * 0.004;
			}
			//-入力を決める
			//状態を更新
			q += Cycle * (gq(q) * mu);
			q[2] = fmod(M_PI * 2 + fmod(q[2], M_PI * 2), M_PI * 2); //角度を0~2πの範囲にする
		}

		// 画面を初期化する
		ClearDrawScreen();
		// プレイヤーを描画する
		printfDx("仮想入力:%s 切り替えはAボタン\n", virtualEnabled ? "ON" : "OFF");
		printfDx("リセットはXボタン\n");
		printfDx("q  = { %4.6f, %4.6f, %4.6f }\n", q[0], q[1], q[2]);
		printfDx("mu = { %4.6f, %4.6f }\n", mu[0], mu[1]);
		printfDx("th = { %4.6f, %4.6f, %4.6f }\n", th[0], th[1], th[2]);
		printfDx("nu = { %4.6f, %4.6f }\n", nu[0], nu[1]);
		DrawCircle(static_cast<int>(th[0] + 320), static_cast<int>(th[1] + 240), 8, GetColor(64, 64, 128));
		DrawLine(static_cast<int>(th[0] + 320), static_cast<int>(th[1] + 240),
			static_cast<int>(320 + th[0] + 16 * cos(th[2])), static_cast<int>(240 + th[1] + 16 * sin(th[2])),
			GetColor(128, 128, 0));
		DrawCircle(static_cast<int>(320 + q[0]), static_cast<int>(240 + q[1]), 8, GetColor(128, 128, 255));
		DrawLine(static_cast<int>(320 + q[0]), static_cast<int>(240 + q[1]),
			static_cast<int>(320 + q[0] + 16 * cos(q[2])), static_cast<int>(240 + q[1] + 16 * sin(q[2])),
			GetColor(255, 255, 0));
		//枠
		DrawBox(static_cast<int>(320 - c1), static_cast<int>(240 - c2),
			static_cast<int>(320 + c1), static_cast<int>(240 + c2), GetColor(255, 0, 0), FALSE);
		// 裏画面の内容を表画面に反映させる
		ScreenFlip();
	}
	DxLib_End(); // ＤＸライブラリ使用の終了処理

	return 0; // ソフトの終了
}

void f()
{}
