#include "../inc/controller.hpp"
#include "../inc/viewer.hpp"
#include "../inc/ui.hpp"
#include "../inc/decoder.hpp"

Controller::Controller() {
	viewer_ = new Viewer(this);
	ui_ = new UI(this);
	decode_status_ = new DecodeStatus();
}

Controller::~Controller() {
	viewer_->~Viewer();
}

int Controller::Display() {
	viewer_->Display();
	return true;
}

int Controller::BuildConnection() {
	ui_->BuildConnection(decode_status_);
	decoder_ = new Decoder(decode_status_);
	return true;
}

int Controller::Disconnection() {
	decoder_->~Decoder();
	decode_status_->visual_status = -1;
	return true;
}