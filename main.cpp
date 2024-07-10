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
public:
	std::vector<Segment> segments;
	int current_seg = 0;
	float seg_sum = 0;
	const float assimilate = 40;
	float acc_curvature = 0;
	float track_curvature = 0;

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
				seg_sum = 0;
			}
			return segments[current_seg].curvature;
		}
	}
};

// Override base class with your custom functionality
class Game : public olc::PixelGameEngine
{
private:
	Car car;
	Track track;

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
		track.add_segment(200, 1.0f);
		track.add_segment(200, 0);
		track.add_segment(200, -1.0f);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(olc::W).bHeld) {
			car.speed += 2.0f * fElapsedTime;
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
		float curvature_diff = (target_curvature - track.acc_curvature) * car.speed * fElapsedTime;
		track.acc_curvature += curvature_diff;
		track.track_curvature += track.acc_curvature * fElapsedTime * car.speed;


		for (int y = ScreenHeight() / 2; y < ScreenHeight(); y++) {
			float perspective = (y - (float)ScreenHeight() / 2.0f) / ((float)ScreenHeight() / 2.0f);
			
			//float perspective = progress + 0.4f;
			//std::cout << progress << '\n';
			int track_width = ScreenWidth() * (perspective * 0.8 + 0.1);
			int edge_width = ScreenWidth() * (perspective * 0.15f + 0.005f);
			//int grass_width = ScreenWidth() - track_width - edge_width * 2;
			int h_track_width = track_width / 2;

			int offset = pow(1.0f - perspective, 3) * track.acc_curvature * ScreenWidth();
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
					color = olc::GREY;
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

		
		int car_pos = ScreenWidth() / 2 + (car.acc_curvature - track.track_curvature) * ScreenWidth() / 2 - car.formula_straight.get()->width/2;
		SetPixelMode(olc::Pixel::Mode::MASK);
		DrawSprite(olc::vi2d(car_pos, 400), car.formula_straight.get());
		SetPixelMode(olc::Pixel::Mode::NORMAL);

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