/*
Исайков Иван Александрович
БПИ 193-2
Микропроект 2
Вариант 7
Программа для моделирования поочерёдных военных действий между 2-мя враждующими странами.

Условие задачи: Военная задача. Анчуария и Тарантерия – два крохотных
латиноамериканских государства, затерянных в южных Андах. Диктатор
Анчуарии, дон Федерико, объявил войну диктатору Тарантерии, дону
Эрнандо. У обоих диктаторов очень мало солдат, но очень много снарядов
для минометов, привезенных с последней американской гуманитарной
помощью. Поэтому армии обеих сторон просто обстреливают наугад
территорию противника, надеясь поразить что-нибудь ценное. Стрельба
ведется по очереди до тех пор, пока либо не будут уничтожены все цели,
либо стоимость потраченных снарядов не превысит суммарную стоимость
всего того, что ими можно уничтожить. Создать многопоточное приложение,
моделирующее военные действия.

Всего входных аргументов: 8 (исключая путь к программе arg[0])
1 - стоимость патрона 1 страны (int)
2 - общая стоимость всех целей в данной стране (int)
3 - количество целей в данной стране (int)
4 - шанс на попадание из миномёта по другой стране (от 0 до 99)
5 - стоимость патрона 2 страны (int)
6 - общая стоимость всех целей в данной стране (int)
7 - количество целей в данной стране (int)
8 - шанс на попадание из миномёта по другой стране (от 0 до 99)

Предполагается, что с программой работает опытный специалист и 
вводит только корректные данные (положительные аргументы в указаных пределах)
В случае ввода иных данных работоспособность программы не гарантируется, как
и правильность вычислений.

Вывод информации о действиях стран (потоков) осуществляется в консоль
*/

#include <iostream>
#include <thread>
#include <mutex>
using namespace std;


/// <summary>
/// Класс представляющий из себя основу для объектов стран
/// </summary>
class Country
{
public:
	Country();
	string name;//имя страны
	int patron_cost;//цена снаряда
	int patron_limit;//суммарная стоимость всего, что можно уничтожить в этой стране
	int cell_number;//колличество целей в данной стране
	bool* cell_array;//массив со значениями bool, определяющими поражение цели
	int hit_chance;//шанс попадания
	int counter;//текущая стоимость всех выпущенных зарядов
	~Country();
};

bool win = false;//выиграна ли война
bool attacker_c;//чья очередь атаковать? (на всякий случай)
Country* A;//первая страна
Country* B;//вторая страна
mutex attacked;//мьютекс блокировки атаки (одновременно может атаковать только 1 страна)

/// <summary>
/// Метод атаки стран (требуется запуск двух "противоборствующих" потоков для корректной работы метода)
/// </summary>
/// <param name="atacker"></param>
/// <param name="enemy"></param>
void attack(Country* atacker, Country* enemy)
{
	//создание сида для rand() из __threadid() и текущего времени для максимально рандомных значений
	srand((unsigned int)(__threadid() + time(NULL)));

	bool first = true;
	bool temp;//переменная хранящая "номер" очереди
	do
	{
		attacked.lock();//блок атаки

		if (first)//для первого раза (чтобы понять кто за кем атакует)
		{
			temp = attacker_c;
			first = false;
			//cout << __threadid() << time(NULL) << endl;
		}
			

		if ((temp == attacker_c) && !win)//если сейчас очередь этой страны и ещё никто не выиграл
		{
#pragma region Attack
			cout << atacker->name << " attack " << enemy->name;
			//main thread code
			atacker->counter++;//выпуск снаряда
			int target = rand() % enemy->cell_number;//выбор рандомной цели
			if (rand() % 100 < atacker->hit_chance - 1)
			{
				bool temp = enemy->cell_array[target];
				enemy->cell_array[target] = true; //цель была поражена
				cout << " and hit the " << target << " point";

				if (temp)//цель уже была поражена
					cout << " again";

				cout << "." << endl;//точка в конце
			}
			else
				cout << " and miss trying to hit " << target << " point." << endl;//не сработал hit_chance (абсолютный промах)
#pragma endregion

#pragma region Check
			bool win_check = true;//проверка на поражение всех целей
			for (int i = 0; i < enemy->cell_number; i++)
				if (!enemy->cell_array[i]) win_check = false;//есть не пораженные цели

			if (win_check)//в случае выигрыша
			{
				win = true;
				cout << atacker->name << " hitted all the " << enemy->name << " targets, and win the war!" << endl;
			}
			else if (atacker->counter * atacker->patron_cost > enemy->patron_limit)//закончились патроны (превышена стоимость)
			{
				win = true;
				cout << atacker->name << " run out of ammo and lose the war!" << endl;
			}
#pragma endregion

			attacker_c = !attacker_c;//смена очереди
		}
		attacked.unlock();//разблокировка прав на атаку

		std::this_thread::sleep_for(1ms);//усыпление потока на всякий случай (время на перезарядку оружия) //без этой команды рограмма тоже работает, но как по мне - слишком быстро (не успеваешь следить за очередностью)
	} while (!win);//пока не будет выявлен победитель/проигравший

	
	return;//возврат обратно в программу
}

/// <summary>
/// Главный программный поток
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, char** argv)
{
	try
	{
		attacker_c = false;
		A = new Country(); B = new Country();//создание обьектов
		A->name = "Anchuria"; B->name = "Taranteria";//имена стран

		A->patron_cost = atoi(((string)argv[1]).c_str());//вместо конструктора класса Country
		A->patron_limit = atoi(((string)argv[2]).c_str());
		A->cell_number = atoi(((string)argv[3]).c_str());
		A->hit_chance = atoi(((string)argv[4]).c_str());
		A->cell_array = new bool[A->cell_number]{ false };

		B->patron_cost = atoi(((string)argv[5]).c_str());
		B->patron_limit = atoi(((string)argv[6]).c_str());
		B->cell_number = atoi(((string)argv[7]).c_str());
		B->hit_chance = atoi(((string)argv[8]).c_str());
		B->cell_array = new bool[B->cell_number]{ false };

		thread Aatack = thread(attack, A, B);//поток атаки 1 страны 
		thread Batack = thread(attack, B, A);//поток атаки 2 страны
		Aatack.join();//ожидание окончания военных действий между потоками
		Batack.join();

		//завершение обоих потоков означает определение победителя => можно завершать программу
		delete A;
		delete B;
		return 0;
	}
	catch (runtime_error ex)
	{
		cout << "Warning! Exception happend! Try to use more correct input data." << endl;
		cout << "Exception message: " << ex.what() << endl;
	}
}

/// <summary>
/// Конструктор класса Country
/// </summary>
Country::Country()
{
	counter = 0;
}

/// <summary>
/// Деструктор класса Country
/// </summary>
Country::~Country()
{
	delete cell_array;
}
