#include "GameScene.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include <cassert>

#include "PrimitiveDrawer.h"
#include "AxisIndicator.h"

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete sprite_;
	delete model_;
	delete debugCamera_;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("uvChecker.png");
	// スプライトの生成
	sprite_ = Sprite::Create(textureHandle_, {100, 100});
	// 3Dモデルの生成
	model_ = Model::Create();

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	// ビュープロジェクションの初期化
	viewProjection_.Initialize();

	// サウンドデータの読み込み
	soundDataHandle_ = audio_->LoadWave("fanfare.wav");
	// 音声再生
	voiceHandle_ = audio_->PlayWave(soundDataHandle_, true);

	// ライン描画が参照するビュープロジェクションを指定する（アドレス渡し）
	PrimitiveDrawer::GetInstance()->SetViewProjection(&viewProjection_);

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);

	// 軸方向表示の表示を有効にする
	AxisIndicator::GetInstance()->SetVisible(true);
	// 軸方向表示が参照するビュープロジェクションを指定する（アドレス渡し）
	AxisIndicator::GetInstance()->SetTargetViewProjection(&debugCamera_->GetViewProjection());
}

void GameScene::Update() {
	Vector2 position = sprite_->GetPosition();
	// 座標移動
	position.x += 2.0f;
	position.y += 1.0f;
	// 移動した座標をスプライトに反映
	sprite_->SetPosition(position);

	// スペースキーを押した瞬間
	if (input_->TriggerKey(DIK_SPACE)) {
		// 再生されていたら
		if (audio_->IsPlaying(voiceHandle_)) {
			// 音声停止
			audio_->StopWave(voiceHandle_);
		} else { // そうでなかったら
			// 再生(voiceHandle_もう一度読み込み)
			voiceHandle_ = audio_->PlayWave(soundDataHandle_, true);
		}
	}

// デバッグテキストの表示
// ImGuiはDebugモード専用(Releaseだとエラー)
#ifdef _DEBUG // Debugモードの時のみ有効にする
	ImGui::Text("Kamata Tarou %d.%d.%d.", 2050, 12, 31);
	ImGui::Begin("Debug1");
	ImGui::Text("Kamata Tarou %d.%d.%d.", 2050, 12, 31);
	ImGui::InputFloat3("InputFloat3", inputFloat3);               // float3入力ボックス
	ImGui::SliderFloat3("SliderFloat3", inputFloat3, 0.0f, 1.0f); // float3スライダー
	ImGui::End();

	ImGui::ShowDemoWindow();
#endif

	// デバッグカメラの更新
	debugCamera_->Update();
}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>
	
	//モデルを連動
	model_->Draw(worldTransform_, debugCamera_->GetViewProjection(), textureHandle_);

	// ライン描画
	PrimitiveDrawer::GetInstance()->DrawLine3d({0, 0, 0}, {0, 10, 0}, {1.0f,0.0f,0.0f,1.0f});
	
	//for文2つ使って格子状描画可能
	/* float lineMeshWidth = 50;
	float lineMeshHeight = 50;
	float lineMeshSpanH = 5;
	float lineMeshSpanV = 5;
	for (float h = lineMeshWidth / 2; h <= lineMeshWidth / 2; h += lineMeshSpanH) {
		PrimitiveDrawer::GetInstance()->DrawLine3d(
			{h,-lineMeshHeight/2,0},
			{h,lineMeshHeight},
		)
	}*/

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>
	
	sprite_->Draw();

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}