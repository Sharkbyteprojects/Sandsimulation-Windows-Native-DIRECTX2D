#include "sandph.hpp"
void Csand::clearsand() {
	for (size_t x = 0; x < sandx; x++) {
		for (size_t y = 0; y < sandy; y++) {
			this->sand[x][y] = false;
		}
	}
}

void Csand::step()
{
	bool ou;
	for (size_t x = 0; x < sandx; x++) {
		for (size_t y2 = 0, y = 0; y2 < (sandy - 1); y2++) {
			y = (sandy - 2) - y2;
			if (this->sand[x][y]) {
				ou = false;
				if (!this->sand[x][y + 1]) {
					this->sand[x][y + 1] = this->sand[x][y];
					ou = true;
				}
				else if (x < sandx - 1 && !this->sand[x + 1][y + 1]) {
					this->sand[x + 1][y + 1] = this->sand[x][y];
					ou = true;
				}
				else if (x > 0 && !this->sand[x - 1][y + 1]) {
					this->sand[x - 1][y + 1] = this->sand[x][y];
					ou = true;
				}
				this->sand[x][y] = !ou;
			}
		}
	}
}
