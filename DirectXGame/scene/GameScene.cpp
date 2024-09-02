#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>

GameScene::GameScene() {}

GameScene::~GameScene() { // デストラクタ
	delete model_; // 3Dモデル
	delete modelSkydome_; // 天球3Dモデル
	delete skydome_; // 天球
	//	delete player_; // プレイヤー
	delete blockModel_; // ブロック3Dモデル
	// 範囲for文で配列内の1個ずつ取り出しながら処理
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock; // 実際にインスタンスを解放する処理
		}
		worldTransformBlocks_.clear(); // 配列から要素を一掃
	}

	delete debugCamera_; // デバッグカメラ
	delete mapChipField_; // マップチップフィールド
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	
	// マップチップ
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();

	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("uvChecker.png");
	// 3Dモデルの生成
	model_ = Model::Create();
	// ビュープロジェクションの初期化
	viewProjection_.farZ = 5000;
	viewProjection_.Initialize();

	// 天球3Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("Skydome", true);
	// 天球の生成
	skydome_ = new Skydome;
	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &viewProjection_);

	// 自キャラの生成
	player_ = new Player();
	
	model_ = Model::CreateFromOBJ("Player", true);
	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);
	// 自キャラの初期化
	player_->Initialise(model_, &viewProjection_, playerPosition);

	// ブロック3Dモデルの生成
	blockModel_ = Model::Create();
	
	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);
}

void GameScene::Update() {
	// 天球の更新
	skydome_->Update();
	
	// 自キャラの更新
	player_->Update();

	// ブロックの更新
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			// アフィン変換行列の作成
			worldTransformBlock->matWorld_ = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

			// 定数バッファに転送する
			worldTransformBlock->TransferMatrix();
		}
	}

// デバッグカメラ
#ifdef _DEBUG
	if (input_->TriggerKey(DIK_BACKSPACE)) {
		isDebugCameraActive_ = !isDebugCameraActive_; // デバッグカメラ有効フラグをトグル
	}
	// カメラの処理
	if (isDebugCameraActive_) {
		debugCamera_->Update(); // デバッグカメラの更新
		
		const ViewProjection& debugViewProjection = debugCamera_->GetViewProjection();
		viewProjection_.matView = debugViewProjection.matView;     // デバッグカメラのビュー行列
		viewProjection_.matProjection = debugViewProjection.matProjection; // デバッグカメラのプロジェクション行列
		
		// ビュープロジェクション行列の転送
		viewProjection_.TransferMatrix();
	} else {
		// ビュープロジェクション行列の更新と転送
		viewProjection_.UpdateMatrix();
	}
#endif

}

void GameScene::GenerateBlocks() {
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVertical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();
	worldTransformBlocks_.resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; i++) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}
	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVertical; i++) {
		for (uint32_t j = 0; j < numBlockHorizontal; j++) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
};

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

	// 天球の描画
	skydome_->Draw();

	// 自キャラの描画
	player_->Draw();

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			blockModel_->Draw(*worldTransformBlock, viewProjection_);
		}
	}

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}
