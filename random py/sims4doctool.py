import flags

class Symptom(flags.Flags):
	rash_tiger = ()
	rash_spotted = ()
	rash_swirl = ()
	ache_stomach = ()
	ache_head = ()
	sneezing = ()
	coughing = ()
	swatting = ()
	giggling = ()
	itching = ()
	dizziness = ()
	steam = ()
	fever = ()

"""
Diseases & Symptoms
	https://www.carls-sims-4-guide.com/forum/index.php?topic=23603.0
	https://sims.fandom.com/wiki/Illness#Get_to_Work_illnesses
"""
class Disease:
	def __init__(self, inc, exc, notes=""):
		self.inclusions = inc
		self.exclusions = exc
		self.notes = notes
	def include(self):
		return self.inclusions
	def exclude(self):
		return self.exclusions
	def note(self):
		self.notes

diseases={}
diseases["Bloaty Head"] = Disease(
	Symptom.ache_head | Symptom.steam,
	Symptom.rash_tiger | Symptom.rash_spotted | Symptom.rash_swirl | Symptom.swatting)

diseases["Gas and Giggles"] = Disease(
	Symptom.rash_tiger | Symptom.ache_stomach  | Symptom.giggling,
	Symptom.rash_spotted | Symptom.rash_swirl)

diseases["Itchy Plumbob"] = Disease(
	Symptom.rash_tiger | Symptom.itching,
	Symptom.rash_spotted | Symptom.rash_swirl)

diseases["Sweaty Shivers"] = Disease(
	Symptom.rash_spotted | Symptom.fever | Symptom.itching,
	Symptom.rash_tiger | Symptom.rash_swirl | Symptom.sneezing | Symptom.coughing)

diseases["Llama Flu"] = Disease(
	Symptom.rash_spotted | Symptom.fever | Symptom.sneezing | Symptom.coughing,
	Symptom.rash_tiger | Symptom.rash_swirl | Symptom.itching)

diseases["Burning Belly"] = Disease(
	Symptom.ache_stomach | Symptom.fever,
	Symptom.rash_tiger | Symptom.rash_spotted  | Symptom.rash_swirl)

diseases["Starry Eyes"] = Disease(
	Symptom.rash_swirl | Symptom.dizziness | Symptom.swatting,
	Symptom.rash_tiger | Symptom.rash_spotted | Symptom.coughing)

diseases["Triple Threat"] = Disease(
	Symptom.rash_spotted | Symptom.rash_swirl | Symptom.dizziness | Symptom.coughing,
	Symptom.rash_tiger | Symptom.swatting | Symptom.fever | Symptom.sneezing)

from tkinter import *
class Checkbar(Frame):
	def __init__(self, parent=None, picks=[]):
		Frame.__init__(self, parent)
		self.boxes = []
		self.vars = []
		for i,pick in enumerate(picks):
			var = IntVar()
			self.boxes.append(Checkbutton(self, text=pick, variable=var))
			self.boxes[len(self.boxes)-1].grid(row=i, sticky="NW")
			self.vars.append(var)
	def state(self):
		return map((lambda var: var.get()), self.vars)
	def box(self):
		return map((lambda box: box), self.boxes)

root=Tk()

symtom_strs = []
for symptom in Symptom:
	symtom_strs.append(str(symptom.to_simple_str()))
	
lng = Checkbar(root, symtom_strs)
lng.pack(side=TOP, fill=X)
lng.config(relief=GROOVE, bd=2)

def differential():
	def collect_symptoms():
		symptoms = 0
		for sym,state in zip(Symptom, list(lng.state())):
			if (state == 1):
				symptoms = symptoms and symptoms | sym or sym
		return Symptom(symptoms)

	def deduction(symptoms):
		diseases_candidates = []
		for key in diseases:
			if ((not diseases[key].exclude() & symptoms) and diseases[key].include() & symptoms):
				diseases_candidates.append(key)
		return diseases_candidates

	symptoms = collect_symptoms()
	print("\n\n===============================")
	print(" Selection: ", (symptoms and symptoms or "None."))
	print(" Differential Diagnoses:\n")
	candidates = deduction(symptoms)
	if (len(candidates) == 0):
		print(" None.")
	else:
		for key in candidates:
			print(' ', key, "\n\t-> ", diseases[key].include(), "\n\n")
	print("\n\n")

def clear():
	for box in lng.box():
		box.deselect()

Button(root, text='Differential', command=differential).pack(side=LEFT)
Button(root, text='Clear', command=clear).pack(side=RIGHT)

root.update()
root.minsize(200, root.winfo_height())
root.mainloop()