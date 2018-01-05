#pragma once
class CameraGain {
	public:
		CameraGain();
		~CameraGain();
		void SetCDSGain(int cds_gain) {cds_gain_ = cds_gain};


	private:
		int gain_mode_, gain_value_;
		int cds_gain_, vga_gain_, black_level_, exposure_time_;

};