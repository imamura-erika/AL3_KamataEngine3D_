#include "Player.h"
#include "cassert"

Player::Player() {}

Player::~Player() {}

void Player::Intialize(Model* model, uint32_t textureHandle) {
	assert(model);
	model_ = model;
	textureHandle_ = textureHandle;
	worldTransform_.Initialize();
}

void Player::Update() {}

void Player::Draw() {}
