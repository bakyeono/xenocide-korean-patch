#ifndef PLACES_H
#define PLACES_H

class PLACES {
private:
	// laboratorium genetyczne
	string taken_DNA;
	void draw_terminal();
	void print_laboratory();
	void print_program_1();
	void print_program_2();
	void print_program_3();
	void print_program_4(int page);
	void print_on_terminal(int x,int y,string text);
	bool GenLabBioRebuild();
	bool GenLabDNAModif();
	bool GenLabIntegration();
	bool GenLabPersonalLog();
	bool IntegratePlayerWith(MONSTER *monster);
public:
	bool GeneticLaboratory();
};

#endif

