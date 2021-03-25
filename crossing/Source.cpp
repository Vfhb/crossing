/*Программа решает очередность проезда машин на перекрестке:
● Если есть помеха справа то машина должна пропустить машину
справа.
● Если на перекрестке стоят машины со всех 4х сторон, то первой
проезжает машина, у которой минимальная координата по X.
● Машины не должны наезжать друг на друга.*/

#include <iostream>
#include <vector>
#include <iomanip>
#include <queue>
#include <ctime>
#include <string>

using namespace std;

/// <summary>
/// Структура координаты. Указывает координату передней центральной точки машины.
/// </summary>
struct sPos
{
	int x;
	int y;
	sPos(int aX = 0, int aY = 0) { x = aX; y = aY; }
};
/// <summary>
/// Размеры машины: длина и ширина.
/// </summary>
struct sSize
{
	int width;
	int length;
	sSize(int aW = 0, int aL = 0) { width = aW; length = aL; }
};
/// <summary>
/// Направлание направления.
/// </summary>
enum class eDirection
{
	UP,
	LEFT,
	RIGHT,
	DOWN
};
/// <summary>
/// Структура, содержащая минимальные и максимальные координаты прямоугольника (отображения машины).
/// </summary>
struct sRect
{
	int xmin, xmax, ymin, ymax;
	sRect() {}
	sRect(sPos pos, sSize size, eDirection dir)
	{
		switch (dir)
		{
		case eDirection::UP:
			xmin = pos.x - size.width / 2; ymin = pos.y - size.length;
			xmax = pos.x + size.width / 2; ymax = pos.y;
			break;
		case eDirection::DOWN:
			xmin = pos.x - size.width / 2; ymin = pos.y;
			xmax = pos.x + size.width / 2; ymax = pos.y + size.length;
			break;
		case eDirection::RIGHT:
			xmin = pos.x - size.length; ymin = pos.y - size.width / 2;
			xmax = pos.x; ymax = pos.y + size.width / 2;
			break;
		case eDirection::LEFT:
			xmin = pos.x; ymin = pos.y - size.width / 2;
			xmax = pos.x - size.length; ymax = pos.y + size.width / 2;
			break;
		}
	}
	bool intersects(const sRect& other)
	{

		return !((xmax < other.xmin) || (other.xmax < xmin) || (ymax < other.ymin) || (other.ymax < ymin));
	}
};
/// <summary>
/// Машина. Абстрактный класс.
/// </summary>
struct sCar
{
	sPos pos;
	sSize size;
	eDirection dir;
	int speed;
	sRect rect;
	sCar(int x = 0, int y = 0, int h = 0, int w = 0, int s = 0, eDirection d = eDirection::UP)
	{
		pos.x = x; pos.y = y; size.length = h; size.width = w; speed = s; dir = d;
		rect = sRect(pos, size, dir);
	}
	/// <summary>
	/// Пересчитывает актуальные координаты прямоугольника для отображения машины.
	/// </summary>
	void calcRect(sPos pos, sSize size, eDirection dir) { rect = sRect(pos, size, dir); }
	virtual void move()
	{
		switch (dir)
		{
		case eDirection::UP:
			pos.y += speed;
			break;
		case eDirection::DOWN:
			pos.y -= speed;
			break;
		case eDirection::RIGHT:
			pos.x += speed;
			break;
		case eDirection::LEFT:
			pos.x -= speed;
			break;
		}
	}
	bool intersects(const sCar& other)
	{
		return this->rect.intersects(other.rect);
	}
	sPos getFuturePos()
	{
		switch (dir)
		{
		case eDirection::UP:
			return sPos(pos.x, pos.y + speed);
		case eDirection::DOWN:
			return sPos(pos.x, pos.y - speed);
		case eDirection::RIGHT:
			return sPos(pos.x + speed, pos.y);
		case eDirection::LEFT:
			return sPos(pos.x - speed, pos.y);
		}
	}
	bool needPassOtherCar(sCar* otherCar)
	{
		bool result = false;
		auto otherdir = otherCar->dir;
		switch (dir)
		{
		case eDirection::UP:
			if (otherdir == eDirection::LEFT)
				result = true;
			break;
		case eDirection::DOWN:
			if (otherdir == eDirection::RIGHT)
				result = true;
			break;
		case eDirection::RIGHT:
			if (otherdir == eDirection::UP)
				result = true;
			break;
		case eDirection::LEFT:
			if (otherdir == eDirection::DOWN)
				result = true;
			break;
		}
		return result;
	}
	/// <summary>
	/// Определяет видимость машины в заданном окне.
	/// </summary>
	bool isVisible(int scrH, int scrW)
	{
		this->calcRect(pos, size, dir);
		return !((rect.xmax < 0) || (scrW < rect.xmin) || (rect.ymax < 0) || (scrH < rect.ymin));
	}
	/// <summary>
	/// Функция возможной отрисовки машины.
	/// </summary>
	virtual void print()
	{
		return;
	}
	virtual int getFuel() = 0;
	virtual void refill(int count) = 0;
};
struct sGasEngine : virtual sCar
{
	int fuel;
	sGasEngine(int f = 2000) { fuel = f; }
	int getFuel() { return fuel; }
	void refill(int count) { fuel += count; }
	void move() { fuel--; sCar::move(); }
	void print() {}
};
struct sElectroCar : virtual sCar
{
	int charge;
	sElectroCar(int c = 2000) { charge = c; }
	int getFuel() { return charge; }
	void refill(int count) { charge += count; }
	void move() { charge--; sCar::move(); }
	void print() {}
};
struct sHybrid : sGasEngine, sElectroCar
{
	sHybrid(int c = 1000, int f = 1000) { charge = c; fuel = f; }
	void refill(int count) { charge += count / 2; fuel += count / 2; }
	int getFuel() { return charge + fuel; }
	void move()
	{
		if (rand() % 2 == 0)
			charge--;
		else
			fuel--;
		sCar::move();
	}
	void print() { }
};

queue <sCar*> TopCars;
queue <sCar*> BotCars;
queue <sCar*> LeftCars;
queue <sCar*> RightCars;
const int initialCarsCount = 10;
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
/// <summary>
/// Количество машин, которое проедет перекресток в данной симуляции.
/// </summary>
const int iterCount = 100;


/// <summary>
/// Функция отбражения количества машин на перекрестке (для удобства проверки).
/// </summary>
/// <param name="action"> - ключевое действие </param>
void print(string action)
{
	cout << action << endl;
	cout << "Bottom: " << BotCars.size() << " Left: " << LeftCars.size() << " Top: " << TopCars.size() << " Right: " << RightCars.size() << endl;
}
void spawnCarFromTop(sCar* car)
{
	car->pos = sPos(SCREEN_WIDTH / 2, SCREEN_HEIGHT);
	car->size = sSize(100, 100);
	car->speed = 1;
	car->dir = eDirection::DOWN;
	TopCars.push(car);
}
void spawnCarFromBot(sCar* car)
{
	car->pos = sPos(SCREEN_WIDTH / 2, 0);
	car->size = sSize(100, 100);
	car->speed = 1;
	car->dir = eDirection::UP;
	BotCars.push(car);
}
void spawnCarFromLeft(sCar* car)
{
	car->pos = sPos(0, SCREEN_HEIGHT / 2);
	car->size = sSize(100, 100);
	car->speed = 1;
	car->dir = eDirection::RIGHT;
	LeftCars.push(car);
}
void spawnCarFromRight(sCar* car)
{
	car->pos = sPos(SCREEN_WIDTH, SCREEN_HEIGHT / 2);
	car->size = sSize(100, 100);
	car->speed = 1;
	car->dir = eDirection::LEFT;
	RightCars.push(car);
}
void spawnCar()
{
	sCar* car;
	int carType = rand();
	if (carType % 3 == 0)
		car = new sGasEngine();
	else if (carType % 3 == 1)
		car = new sElectroCar();
	else
		car = new sHybrid();

	int spawnDir = rand();
	if (spawnDir % 4 == 0)
		spawnCarFromRight(car);
	else if (spawnDir % 4 == 1)
		spawnCarFromTop(car);
	else if (spawnDir % 4 == 2)
		spawnCarFromBot(car);
	else
		spawnCarFromLeft(car);
}
void main_loop()
{
	//проверка, если перекресток пуст
	if ((TopCars.empty()) && (BotCars.empty()) && (LeftCars.empty()) && (RightCars.empty()))
	{
		spawnCar();
		return;
	}

	sCar* MovingCar;
	//изначальная инициализация машины, которая будет ехать в данную итерацию
	if (!TopCars.empty())
		MovingCar = TopCars.front();
	else if (!BotCars.empty())
		MovingCar = BotCars.front();
	else if (!LeftCars.empty())
		MovingCar = LeftCars.front();
	else
		MovingCar = RightCars.front();

	//если стоят машины со всех направлений - преимущество у машины с наименьшей координатой: у нижней.
	if (!(TopCars.empty()) && !(BotCars.empty()) && !(LeftCars.empty()) && !(RightCars.empty()))
		MovingCar = BotCars.front();
	//проверка правил приоритета
	else
	{
		if (!(TopCars.empty()))
			MovingCar = TopCars.front();
		if (!(BotCars.empty()))
			if (MovingCar->needPassOtherCar(BotCars.front()))
				MovingCar = BotCars.front();
		if (!(LeftCars.empty()))
			if (MovingCar->needPassOtherCar(LeftCars.front()))
				MovingCar = LeftCars.front();
		if (!(RightCars.empty()))
			if (MovingCar->needPassOtherCar(RightCars.front()))
				MovingCar = RightCars.front();
	}

	//убираем текущую машину из очереди
	auto moveDir = MovingCar->dir;
	switch (moveDir)
	{
	case eDirection::UP:
		BotCars.pop();
		break;
	case eDirection::DOWN:
		TopCars.pop();
		break;
	case eDirection::RIGHT:
		LeftCars.pop();
		break;
	case eDirection::LEFT:
		RightCars.pop();
		break;
	}

	//машина переезжает перекресток, пока не проедет экран видимости
	while (MovingCar->isVisible(SCREEN_HEIGHT, SCREEN_WIDTH))
		MovingCar->move();

	print("MOVE");

	spawnCar();

	print("SPAWN");
	cout << endl;

	return;
}
int main(int argc, char** argv)
{
	srand(time(NULL));
	for (int i = 0; i < initialCarsCount; ++i)
		spawnCar();

	print("START");
	cout << endl;

	for (int i = 1; i <= iterCount; i++)
	{
		cout << '#' << i << endl;
		main_loop();
	}
	return 0;
}