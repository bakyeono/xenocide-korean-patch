#include "monster.h"
#include "map.h"
#include "keyboard.h"
#include "actions.h"
#include "system.h"
#include "global.h"
#include "directions.h"
#include "options.h"
#include "attrib.h"
#include "level.h"
#include "places.h"
#include "parser.h"

extern GAME game;
extern KEYBOARD keyboard;
extern LEVEL level;
extern MYSCREEN screen;
extern DEFINITIONS definitions;
extern class OPTIONS options;

void PLACES :: print_on_terminal(int x, int y, string text)
{
	for (size_t a=0;a<text.size();a++)
	{
		char character = text[a];
		if (character!=' ')
			set_color(10);
		else
		{
			set_color(2);
			character='.';
		}
		print_character(x+4+a,y+4,character);
	}
}

void PLACES :: draw_terminal()
{
	screen.clear_all();
	int x,y;
	int sizex=57,sizey=21;
	for (x=2;x<=sizex;x++)
	{
		for (y=2;y<=sizey;y++)
		{
			if (x==2 || x==sizex || y==2 || y==sizey)
				set_color(2);
			else
				set_color(2);
			
			print_character(x,y,'.');
		}
	}
	
	set_color(8);
	for (x=3;x<=sizex+1;x++)
		print_character(x,sizey+1,'.');
	for (y=3;y<=sizey+1;y++)
		print_character(sizex+1,y,'.');	

}
void PLACES :: print_laboratory()
{
	draw_terminal();
	
	set_color(10);
				
	int px,py;
	px = 0;
	py = 0;
	string text;
	print_on_terminal(px,py++,"홀로그램 터미널 ver.0.7a");	
	py++;
	switch (random(5))
	{
	case 0: text="박사님, 안녕하세요.";
		break;
	case 1: text="박사님, 또 오셨네요.";
		break;
	case 2: text="박사님, 오랜만이에요.";
		break;
	case 3: text="박사님, 반가워요.";
		break;
	case 4: text="박사님, 명령을 내려주세요.";
		break;
	}
	print_on_terminal(px,py++,text);
	switch (random(5))
	{
	case 0: text="명령이 무엇입니까?";
		break;
	case 1: text="어떤 프로그램을 실행합니까?";
		break;
	case 2: text="가능한 프로젝트:";
		break;
	case 3: text="선택해주세요:";
		break;
	case 4: text="무엇을 열까요?";
		break;
	}
	print_on_terminal(px,py++,text);
	py++;
	print_on_terminal(px,py++,">1.생물체 재설계 7");
	print_on_terminal(px,py++,">2.DNA 조작");
	print_on_terminal(px,py++,">3.DI 227BS - DNA 결합");
	print_on_terminal(px,py++,">4.개인 일지");

	myrefresh();
}
	
bool PLACES::GeneticLaboratory()
{
	taken_DNA = "";
	print_laboratory();
	while(1)
	{
		int key = keyboard.wait_for_key();
		if (key == keyboard.escape || key == keyboard.readmore || key == keyboard.readmore2)
		{
			break;
		}		
		else if (key == '1')
		{
			if (GenLabBioRebuild()==true)
				return true;
			print_laboratory();
		}
		else if (key == '2')
		{
			if (GenLabDNAModif()==true)
				return true;
			print_laboratory();
		}
		else if (key == '3')
		{
			if (GenLabIntegration()==true)
				return true;
			print_laboratory();
		}
		else if (key == '4')
		{
			GenLabPersonalLog();
			print_laboratory();
		}
	}
	return false;
}

void PLACES::print_program_1()
{
	int px,py;
	px=0,py=0;

	draw_terminal();
	print_on_terminal(px,py++,"-= 생물체 재설계 프로그램 =- ver. 7.12.24");
	py++;
	print_on_terminal(px,py++,"최종 수정: 2081-02-05");
	py++;
	print_on_terminal(px,py++,"자가 진단... CRC OK.");
	py++;
	print_on_terminal(px,py++,"하드웨어 점검:");
	py++;
	print_on_terminal(px,py++,"생물체 생성기 시험중... OK.");
	print_on_terminal(px,py++,"DNA 추출기 시험중... OK.");
	print_on_terminal(px,py++,"하드웨어 점검 완료.");
	py++;
	py++;
	print_on_terminal(px,py++,"사용자 옵션:");
	print_on_terminal(px,py++,">1.DNA를 사용한 생물체 재설계.");
	
	myrefresh();
}

void PLACES::print_program_2()
{
	int px,py;
	px=0,py=0;

	draw_terminal();
	print_on_terminal(px,py++,"오류: 설정 파일이 존재하지 않습니다.");
	print_on_terminal(px,py++,"새로운 설정 파일을 업로드하십시오.");
	myrefresh();
}

void PLACES::print_program_3()
{
	int px,py;
	px=0,py=0;
	
	draw_terminal();
	print_on_terminal(px,py++,"실험 DI 227BS");
	print_on_terminal(px,py++,"DNA 결합");
	py++;
	if (taken_DNA=="")
		print_on_terminal(px,py++,"실험을 위한 DNA 샘플이 존재하지 않습니다.");
	else
		print_on_terminal(px,py++,"DNA 추출 " + taken_DNA);
	py++;
	print_on_terminal(px,py++,">1.DNA를 추출할 생물체 선택.");
	if (taken_DNA=="")
		print_on_terminal(px,py++," 2.살아 있는 생물체에 추출한 DNA 결합.");
	else
		print_on_terminal(px,py++,">2.살아 있는 생물체에 추출한 DNA 결합.");

	myrefresh();
}

void PLACES::print_program_4(int page)
{
	int px,py;
	px=0,py=0;

	draw_terminal();

	switch(page) {
	case 1:
		print_on_terminal(px,py++,"[ED 06-12-2189]");
		print_on_terminal(px,py++,"드디어 성공이다! 유전자기계도 잘 작동하고,");
		print_on_terminal(px,py++,"DNA 합성도 잘 이루어지고 있다. 예상대로다.");
		print_on_terminal(px,py++,"이제 화물선이 도착할 때까지 기다리는 일만 남았다.");
		print_on_terminal(px,py++,"화물선 이름이 뭐였지? \"아틀라스\"라고 했던가?");
		print_on_terminal(px,py++,"제발 빨리만 도착해 다오.");
		print_on_terminal(px,17,">2. 다음 쪽");
		break;
	case 2:
		print_on_terminal(px,py++,"[ED 08-12-2189] (1/2)");
		print_on_terminal(px,py++,"연구할 수 있는 샘플이라곤 하나도 안 남았는데");
		print_on_terminal(px,py++,"연구 결과를 보고하라고? 젠텍 놈들 제정신이 아니군!");
		print_on_terminal(px,py++,"화물선이 빨리 도착해야 할텐데.");
		print_on_terminal(px,py++,"명목상으로는 광물을 들여오는 거지만, 실제로는");
		print_on_terminal(px,py++,"그 속에 실험을 위한 외계 생물체들이 들어 있다.");
		print_on_terminal(px,py++,"그러면 내 DNA 실험을 계속 할 수 있겠지!");
		print_on_terminal(px,16,">1. 앞 쪽");
		print_on_terminal(px,17,">2. 다음 쪽");
		break;
	case 3:
		print_on_terminal(px,py++,"[ED 08-12-2189] (2/2)");		
		print_on_terminal(px,py++,"난 기다리기 지쳤다. 멍청한 광부들 중 실험을 도울");
		print_on_terminal(px,py++,"사람을 찾아 보아야 할까? 아냐, 그놈들이 내 연구가");
		print_on_terminal(px,py++,"얼마나 중요한지를 이해할 턱이 있나. 한치 앞도 내다보지");
		print_on_terminal(px,py++,"못하는 멍청이들! 최근 들어서 몇 가지 의문사가");
		print_on_terminal(px,py++,"발생했는데, 그놈들은 내 실험에서 생성된 생물체가");
		print_on_terminal(px,py++,"원인이라고 의심한다. 내가 실험체들을 잘 가둬서 엄격하게");
		print_on_terminal(px,py++,"관리하고 있다고 그렇게 설명을 했는데도 내 말을 믿지");
		print_on_terminal(px,py++,"않는다. 의문사의 원인을 나에게서만 찾다니, 멍청한");
		print_on_terminal(px,py++,"광부들 같으니라고. 자기들 중에 피에 환장한 연쇄 살인범이");
		print_on_terminal(px,py++,"있을지 누가 알겠나.");
		print_on_terminal(px,16,">1. 앞 쪽");
		break;
	default:;
	}	
	myrefresh();
}


bool PLACES::GenLabBioRebuild()
{
	print_program_1();
	while(1)
	{
		int key = keyboard.wait_for_key();
		if (key == keyboard.escape || key == keyboard.readmore || key == keyboard.readmore2)
		{
			break;
		}		
		else if (key == '1')
		{
			level.player->hit_points.val = level.player->hit_points.max;

			for (int a=0;a<NUMBER_OF_STATES;a++)
				level.player->state[a]=0;

			screen.console.add("테이블 위에 눕는다. 기계가 주사를 놓는다. 긴 꿈을 꾸기 시작한다...",7);
			screen.console.add("악몽을 꾼다... 한참동안 긴 시간이 지난다. 몇 시간이 지났는지 알 수 없다. 지금은 아주 상쾌한 기분이다.",7);

			level.player->wait(random(200)+100);				

			level.is_player_on_level = false;
			level.monsters.remove(level.player);
			level.map.setBlockMove(level.player->pX(),level.player->pY());
			
			return true;
		}
	}		
	return false;
}

bool PLACES::GenLabDNAModif()
{
	print_program_2();
	while(1)
	{
		int key = keyboard.wait_for_key();
		if (key == keyboard.escape || key == keyboard.readmore || key == keyboard.readmore2)
		{
			break;
		}		
	}		
	return false;
}


bool PLACES::GenLabIntegration()
{
	int character;
	ITEM *temp;
	CORPSE *cialo;
	MONSTER *monster;
	string text;
	int zwracana;

	print_program_3();
	while(1)
	{
		int key = keyboard.wait_for_key();

		if (key == keyboard.escape || key == keyboard.readmore || key == keyboard.readmore2)
		{
			break;
		}		
		else if (key == '1')
		{
			level.player->show_backpack(NULL,INV_SHOW_HIGHLIGHT_CORPSES);
            set_color(10);
			text = " 어느 시체에서 DNA를 추출합니까? ";
			level.player->adaptation_points = 6;
            print_text(40-text.size()/2,0,text);
			myrefresh();
            while (1)
            {
				character=keyboard.wait_for_key();
				if ((character>='a' && character<='z') || (character>='A' && character<='Z'))
				{
					temp=level.player->choose_item_from_backpack(character);
					if (temp!=NULL) // gdy cos wybrano
					{
						cialo = temp->IsCorpse();
						if (cialo!=NULL)
						{
							taken_DNA = cialo->of_what;
							cialo->death();
							print_program_3();
							break;							
						}
					}
				}
				else if (character==keyboard.escape || character==keyboard.readmore || character==keyboard.readmore2)
				{
					print_program_3();
					break;
				}				
			}
		}		
		else if (key == '2')
		{
			if (taken_DNA!="")
			{
				monster = definitions.find_monster(taken_DNA);

				draw_terminal();
				int px=0,py=0;
				print_on_terminal(px,py++,"박사님!");
				print_on_terminal(px,py++,"이곳에 있는 생물은 당신 뿐입니다!");
				print_on_terminal(px,py++,"정말로 당신을 합성 재료로 사용합니까?");
				py++;
				print_on_terminal(px,py++,"(y/n)");
				py++;
				print_on_terminal(px,py++,"(위험성을 깨달으셨습니까?)");
				myrefresh();
				zwracana=keyboard.wait_for_key();
				if (zwracana!='y' && zwracana!='Y')
				{
					print_program_3();
					break;							
				}
				print_on_terminal(6,4,"Y");
				myrefresh();
				if (level.player->adaptation_points<1)
				{
					print_on_terminal(px,py++,"당신의 조직에는 이 DNA를 적용할 수 없습니다.");
					myrefresh();
					keyboard.wait_for_key();
					print_program_3();
					break;							
				}
				print_on_terminal(px,py++,"시작합니다...");
				delay(150);
				myrefresh();
				
				if (IntegratePlayerWith(monster)==false)
					return false;

				level.player->wait(random(500)+500);				
				level.is_player_on_level = false;
				level.monsters.remove(level.player);
				level.map.setBlockMove(level.player->pX(),level.player->pY());				
				return true;
			}
		}		
	}
	return false;
}


#define LOG_PAGES 3

bool PLACES::GenLabPersonalLog()
{
	int page = 1;
	print_program_4(page);
	while(1)
	{
		int key = keyboard.wait_for_key();
		if (key == '1' && page>1)
			page--;
		else if (key == '2' && page<LOG_PAGES)
			page++;		
		else if (key == keyboard.escape || key == keyboard.readmore || key == keyboard.readmore2)
		{
			break;
		}	
		print_program_4(page);		
	}		
	return false;
}

bool PLACES::IntegratePlayerWith(MONSTER *monster)
{
	if (monster==NULL)
		return false;
		
	level.player->adaptation_points--;

	screen.console.add(" \"실험: DNA 합성을 시작합니다.\".\n",11);		
	screen.console.add("잠이 든다...\n",7);		
	screen.console.add("... ... ... ...\n",7);		
	screen.console.add("한참이 지난 후 잠에서 깨어난다.\n",7);		
	
	int random_value;

	int new_value;
	// power	

	if (level.player->strength.val < monster->strength.val)
		screen.console.add("힘이 강해진 느낌이다...",7);
	else if (level.player->strength.val > monster->strength.val)
		screen.console.add("힘이 약해진 느낌이다...",7);
	
	new_value = (level.player->strength.max + monster->strength.max)/2;
	level.player->strength.val += new_value - level.player->strength.max;
	level.player->strength.max = new_value;
	
	// zrecznosc
	if (level.player->dexterity.val < monster->dexterity.val)
		screen.console.add("몸이 유연해진 느낌이다...",7);
	else if (level.player->dexterity.val > monster->dexterity.val)
		screen.console.add("몸이 경직된 느낌이다...",7);

	new_value = (level.player->dexterity.max + monster->dexterity.max)/2;
	level.player->dexterity.val += new_value - level.player->dexterity.max;
	level.player->dexterity.max = new_value;
	
	// wytrzymalosc
	if (level.player->endurance.val < monster->endurance.val)
		screen.console.add("몸이 강인해진 느낌이다...",7);
	else if (level.player->endurance.val > monster->endurance.val)
		screen.console.add("몸이 허약해진 느낌이다...",7);

	new_value = (level.player->endurance.max + monster->endurance.max)/2;
	level.player->endurance.val += new_value - level.player->endurance.max;
	level.player->endurance.max = new_value;
	
	// inteligencja
	if (level.player->intelligence.val < monster->intelligence.val)
		screen.console.add("사고가 명확해졌다...",7);
	else if (level.player->intelligence.val > monster->intelligence.val)
		screen.console.add("집중이 잘 안 된다...",7);

	new_value = (level.player->intelligence.max + monster->intelligence.max)/2;
	level.player->intelligence.val += new_value - level.player->intelligence.max;
	level.player->intelligence.max = new_value;
	
	// szybko쒏
	if (level.player->speed.val < monster->speed.val)
		screen.console.add("행동이 재빨라졌다...",7);
	else if (level.player->intelligence.val > monster->intelligence.val)
		screen.console.add("행동이 둔해졌다...",7);
	
	new_value = (level.player->speed.max + monster->speed.max)/2;
	level.player->speed.val += new_value - level.player->speed.max;
	level.player->speed.max = new_value;
	
	// hit points

	if (level.player->hit_points.max < monster->hit_points.max)
		screen.console.add("생명력이 증가한 느낌이다...",7);
	else if (level.player->hit_points.max > monster->hit_points.max)
		screen.console.add("생명력이 감소한 느낌이다...",7);
	
	new_value = (level.player->hit_points.max + monster->hit_points.max)/2;
	level.player->hit_points.val += new_value - level.player->hit_points.max;
	level.player->hit_points.max = new_value;

	if (level.player->hit_points.val<=0)
		level.player->hit_points.val=1;
	
	// metabolizm
	random_value = level.player->metabolism.val - monster->metabolism.val;
	
	if (random_value>0) // player szybszy
	{
		if (random_value>4)
			random_value=4;
		
		random_value = random(random_value-1)+1;
		level.player->metabolism.val-=random_value;
		level.player->metabolism.val-=random_value;
	}
	else if (random_value<0) // player wolniejszy
	{
		if (random_value<4)
			random_value=-4;
		
		random_value = random(random_value-1)+1;
		level.player->metabolism.val+=random_value;
		level.player->metabolism.val+=random_value;
	}
	// skora
	bool change = false;
	if (level.player->no_armor.name != monster->no_armor.name)
	{
		if (level.player->no_armor.ARM < monster->no_armor.ARM)
		{
			level.player->no_armor.ARM++;
			change = true;
		}
		else if (level.player->no_armor.ARM > monster->no_armor.ARM)
		{
			level.player->no_armor.ARM--;
			change = true;
		}
		else // ARM jest takie samo, nabiera properties
		{
			for (PROPERTIES a=1;a!=TYPE_LAST_UNUSED;a+=a)
			{
				if (monster->no_armor.properties&a) // gdy ma ceche
				{
					if (!level.player->no_armor.properties&a) // gdy cechy nie ma
						if (random(3)==0)
						{
							level.player->no_armor.properties^=a; // ustawiamy bit
							change = true;						
							break;
						}
				}
				else if (!monster->no_armor.properties&a) // gdy nie ma cechy
				{
					if (level.player->no_armor.properties&a) // gdy ma ceche
						if (random(3)==0)
						{
							level.player->no_armor.properties^=a; // zerujemy w ten sposob dany bit
							change = true;						
							break;
						}
				}
			}			
		}
		if (monster->no_armor.ARM == level.player->no_armor.ARM &&
			monster->no_armor.properties == level.player->no_armor.properties)
		{
			screen.console.add(level.player->no_armor.name + "이 " + monster->no_armor.name + "으로 변형되었다.",7);			
			level.player->no_armor.name = monster->no_armor.name;
			level.player->no_armor.color = monster->no_armor.color;
		}
		else if (change)
			screen.console.add(level.player->no_armor.name + "이 " + monster->no_armor.name + "으로 변형되었다.",7);

	}
	// gole rece
	change = false;
	if (level.player->unarmed.name != monster->unarmed.name)
	{
		if (level.player->unarmed.DEF < monster->unarmed.DEF)
		{
			level.player->unarmed.DEF++;
			change = true;
		}
		else if (level.player->unarmed.DEF > monster->unarmed.DEF)
		{
			level.player->unarmed.DEF--;
			change = true;
		}

		if (level.player->unarmed.HIT < monster->unarmed.HIT)
		{
			level.player->unarmed.HIT++;
			change = true;
		}
		else if (level.player->unarmed.HIT > monster->unarmed.HIT)
		{
			level.player->unarmed.HIT--;
			change = true;
		}

		if (level.player->unarmed.DMG < monster->unarmed.DMG)
		{
			level.player->unarmed.DMG++;
			change = true;
		}
		else if (level.player->unarmed.DMG > monster->unarmed.DMG)
		{
			level.player->unarmed.DMG--;
			change = true;
		}
		
		for (PROPERTIES a=1;a<=TYPE_LAST_UNUSED;a+=a)
		{
			if (monster->unarmed.properties&a) // gdy ma ceche
			{
				if ((level.player->unarmed.properties&a)==0) // gdy cechy nie ma
				{
					change = true;						
					if (random(5)==0)
					{
						level.player->unarmed.properties^=a; // ustawiamy bit
						break;
					}
				}
			}
			else if (!monster->unarmed.properties&a) // gdy nie ma cechy
			{
				if ((level.player->unarmed.properties&a)==0) // gdy cechy nie ma
				{
					change = true;						
					if (random(5)==0)
					{
						level.player->unarmed.properties^=a; // zerujemy w ten sposob dany bit
						break;
					}
				}
			}
		}
		if (change)
			screen.console.add(level.player->unarmed.name + "은 변이를 일으킨다...",7);		
		else
		{
			screen.console.add(level.player->unarmed.name + "은 " + monster->unarmed.name + "으로 변형되었다!",7);		
			level.player->unarmed.name = monster->unarmed.name;
			level.player->unarmed.color = monster->unarmed.color;
		}
		
	}

	// resisty

	for (int b=0;b<NUMBER_OF_RESISTS;b++)
	{
		if (level.player->resist[b]<monster->resist[b])
			level.player->resist[b] += random(min(3,monster->resist[b] - level.player->resist[b]));
		else if (level.player->resist[b]>monster->resist[b])
			level.player->resist[b] -= random(min(3,level.player->resist[b] - monster->resist[b]));
	}
	
	level.player->set_weapon(&level.player->unarmed);
	level.player->set_armor(&level.player->no_armor);
	
	screen.console.add("\n \"DNA 합성이 완료되었습니다.\".",11);			
	return true;
}

