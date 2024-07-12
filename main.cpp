#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <memory>
#include <iostream>
#include <vector>

class Game;

class Car {
public:
	static std::unique_ptr<olc::Sprite> formula_straight;
	static std::unique_ptr<olc::Sprite> formula_left;
	static std::unique_ptr<olc::Sprite> formula_right;
	//static olc::Sprite* formula_straight;

	static void load_sprites() {
		Car::formula_straight = std::make_unique<olc::Sprite>("./Formula1.png");
		Car::formula_left = std::make_unique<olc::Sprite>("./Formula2.png");
		Car::formula_right = std::make_unique<olc::Sprite>("./Formula3.png");
	}

	float travelled = 0;
	float speed = 0;
	float acc_curvature = 0;
	float offset = 0;


	//void move(float t, float a) {
	//	//travelled += 0.5f * a * t * t + speed * t;
	//	speed += a * t;

	//	if (speed < 0)
	//		speed = 0;

	//	if (speed > 1.0f)
	//		speed = 1.0f;


	//	travelled += speed * t * 70;

	//}

};

std::unique_ptr<olc::Sprite> Car::formula_straight = nullptr;
std::unique_ptr<olc::Sprite> Car::formula_left = nullptr;
std::unique_ptr<olc::Sprite> Car::formula_right = nullptr;


struct Segment {
	float length;
	float curvature;
};

class Track {
private:
	bool lap_end = false;
public:
	std::vector<Segment> segments;
	int current_seg = 0;
	float seg_sum = 0;
	const float assimilate = 40;
	float display_curvature = 0;
	float game_curvature = 0;

	void add_segment(float length, float curvature) {
		segments.push_back({ length, curvature });
	}

	float get_curvature(float travelled) {
		if (travelled < segments[current_seg].length + seg_sum) {
			return segments[current_seg].curvature;
		}
		//else if (travelled < segments[current_seg].length + seg_sum + assimilate) {
		//	float leftover = travelled - (segments[current_seg].length + seg_sum);
		//	float perc = leftover / assimilate;
		//	return (1 - perc)* segments[current_seg].curvature + perc * segments[(current_seg + 1) % segments.size()].curvature;
		//}
		else {
			seg_sum += segments[current_seg].length;
			current_seg++;
			if (current_seg == segments.size()) {
				current_seg = 0;
				//seg_sum = 0;
				lap_end = true;
			}
			return segments[current_seg].curvature;
		}
	}

	bool ended_lap() {
		if (lap_end) {
			lap_end = false;
			return true;
		}
		return false;
	}

	bool is_last_segment() {
		return current_seg == segments.size() - 1;
	}
};

class LapManager {
public:
	float laps[5] = {};
	int current_lap = 0;
	float total_time = 0;

	std::string str() {
		std::string result = "";
		for (int i = 0; i < 5; i++) {
			if (i < current_lap) {
				result += std::to_string(i+1) + ". " + std::to_string(laps[i]) + '\n';
			}
			else if (i == current_lap) {
				result += std::to_string(i + 1) + ". " + std::to_string(total_time) + '\n';
			}
			else {
				result += std::to_string(i + 1) + ". " + "---\n";
			}
		}
		return result;
	}

	void set() {
		laps[current_lap++] = total_time;
	}
};

struct Sine {
	float period;
	float phase;
};

class Mountain {
	std::vector<Sine> sines;

public:
	void add_sine(float period=1, float phase=0) {
		sines.push_back({ period, phase });
	}

	float get(float x) {
		/*Sums all sines and returns value between(0, 1)*/
		float value = 0;
		for (int i = 0; i < sines.size(); i++) {
			value += sinf(sines[i].period * x + sines[i].phase);
		}
		
		return 1 / (1 + exp(-value));
	}
};

// Override base class with your custom functionality
class Game : public olc::PixelGameEngine
{
private:
	Car car;
	Track track;
	Mountain mountain;
	LapManager lapManager;
	bool gameover = false;

public:
	Game()
	{
		// Name your application
		sAppName = "Example";
	}

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		Car::load_sprites();

		//track.add_segment(100, 0);
		//track.add_segment(200, 1.0f);
		track.add_segment(200, 0);
		//track.add_segment(200, -1.0f);
		track.add_segment(50, 0);
		//track.add_segment(10, 0);

		mountain.add_sine(0.5f);
		mountain.add_sine(2);
		mountain.add_sine(4);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (gameover) {
			fElapsedTime = 0;
		}

		// --------------UPDATING---------------
		lapManager.total_time += fElapsedTime;

		if (GetKey(olc::W).bHeld) {
			car.speed += 2.0f * fElapsedTime;
		}

		if (abs(car.acc_curvature - track.game_curvature) > 0.8f) {
			car.speed -= 5.0f * fElapsedTime;
		}
		else {
			car.speed -= 1.0f * fElapsedTime;
		}

		if (GetKey(olc::A).bHeld) {
			car.acc_curvature -= 0.7f * (1 - car.speed / 2) * fElapsedTime;
		}

		if (GetKey(olc::D).bHeld) {
			car.acc_curvature += 0.7f * (1 - car.speed / 2) * fElapsedTime;
		}
		
		if (car.speed < 0)
			car.speed = 0;

		if (car.speed > 1.0f)
			car.speed = 1.0f;

		car.travelled += car.speed * fElapsedTime * 70.0f;

		float target_curvature = track.get_curvature(car.travelled);
		float curvature_diff = (target_curvature - track.display_curvature) * car.speed * fElapsedTime;
		track.display_curvature += curvature_diff;
		track.game_curvature += track.display_curvature * fElapsedTime * car.speed;

		if (track.ended_lap()) {
			//TODO: Special drawing
			lapManager.set();
			if (lapManager.current_lap == 5) {
				gameover = true;
			}
		}

		// --------------DRAWING---------------

		// Draw Sky
		for (int x = 0; x < ScreenWidth(); x++) {
			for (int y = 0; y < ScreenHeight() / 4; y++) {
				Draw(olc::vi2d(x, y), olc::DARK_BLUE);
			}
		}

		// Draw Mountains
		for (int x = 0; x < ScreenWidth(); x++) {
			float h = mountain.get(x * 0.01f + track.game_curvature*5.0f) - 0.1f;
			h *= ScreenHeight() / 4;
			for (int y = ScreenHeight() / 4; y < ScreenHeight() / 2; y++) {
				olc::Pixel color = (ScreenHeight()/2 - y) > h ? olc::BLUE : olc::DARK_RED;
				Draw(olc::vi2d(x, y), color);
			}
		}


		// Draw Track
		for (int y = ScreenHeight() / 2; y < ScreenHeight(); y++) {
			float perspective = (y - (float)ScreenHeight() / 2.0f) / ((float)ScreenHeight() / 2.0f);
			
			//float perspective = progress + 0.4f;
			//std::cout << progress << '\n';
			int track_width = ScreenWidth() * (perspective * 0.8 + 0.1);
			int edge_width = ScreenWidth() * (perspective * 0.15f + 0.005f);
			//int grass_width = ScreenWidth() - track_width - edge_width * 2;
			int h_track_width = track_width / 2;

			int offset = pow(1.0f - perspective, 3) * track.display_curvature * ScreenWidth();
			int midpoint = ScreenWidth() / 2 + offset;

			/*int p1 = grass_width / 2;
			int p2 = p1 + edge_width;
			int p3 = p2 + track_width;
			int p4 = p3 + edge_width;*/

			int p2 = midpoint - h_track_width;
			int p1 = p2 - edge_width;
			int p3 = midpoint + h_track_width;
			int p4 = p3 + edge_width;

			float alternator_edge = sin(pow(1.0f-perspective, 2)*80.0f + car.travelled );
			float alternator_grass = sin(pow(1.0f-perspective, 3)*20.0f + car.travelled * 0.1f);


			for (int x = 0; x < ScreenWidth(); x++) {
				olc::Pixel color;

				if (x < p1) {
					color = alternator_grass > 0 ? olc::GREEN : olc::DARK_GREEN;
				}
				else if (x < p2) {
					color = alternator_edge > 0 ? olc::WHITE : olc::RED;
				}
				else if (x < p3) {
					if (track.is_last_segment()) {
						float podiel = (car.travelled-track.seg_sum) / track.segments[track.current_seg].length;
						float delta = perspective*0.3;//pow(perspective, 3);
						if (perspective - delta < podiel && perspective > podiel) {
							float r = sinf(x * 0.3f) * sinf(y * 0.3f - car.travelled);
							color = r < 0 ? olc::GREY : olc::BLACK;
							//unsigned r = (unsigned) (x * 0.1 - y * 0.1 + car.travelled * 0.1) % 2;
							//color = r == 0 ? olc::GREY : olc::BLACK;
						}
						else {
							color = olc::GREY;
						}
						//unsigned r = (x + y + (unsigned)car.travelled) % 2;
					}
					else {
						color = olc::GREY;
					}
				}
				else if (x < p4) {
					color = alternator_edge > 0 ? olc::WHITE : olc::RED;
				}
				else {
					color = alternator_grass > 0 ? olc::GREEN : olc::DARK_GREEN;
				}

				Draw(olc::vi2d(x, y), color);
			}
		}

		

		olc::Sprite* sprite;
		if (GetKey(olc::A).bHeld)
			sprite = car.formula_left.get();
		else if (GetKey(olc::D).bHeld)
			sprite = car.formula_right.get();
		else
			sprite = car.formula_straight.get();

		int car_pos = ScreenWidth() / 2 + (car.acc_curvature - track.game_curvature) * ScreenWidth() / 2 - sprite->width / 2;
		SetPixelMode(olc::Pixel::Mode::MASK);
		DrawSprite(olc::vi2d(car_pos, ScreenHeight() - sprite->height - 25), sprite);
		SetPixelMode(olc::Pixel::Mode::NORMAL);

		std::string debug_str = std::format("Distance travelled: {}\nCar cuvature: {}\nTrack curvature: {}", car.travelled, car.acc_curvature, track.game_curvature);
		DrawString(olc::vi2d(0, 0), debug_str);

		DrawString(olc::vi2d(10, 40), lapManager.str());

		if (gameover) {
			DrawString(olc::vi2d(ScreenWidth() / 2 - 180, ScreenHeight() / 2 - 100), "GAME OVER", olc::WHITE, 5U);
		}
		

		return true;
	}
};



int main()
{
	Game demo;
	if (demo.Construct(600, 600, 1, 1))
		demo.Start();
	return 0;
}