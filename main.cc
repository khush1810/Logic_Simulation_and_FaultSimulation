// ESE-549 Project 2 
/* Submitted by 
Pranith Kumar Thadagoppula (111447383)
Kushboo Kumari (111634864)
*/

#include <iostream>
#include <fstream> 
#include <vector>
#include <queue>
#include <time.h>
#include <stdio.h>
#include "parse_bench.tab.h"
#include "ClassCircuit.h"
#include "ClassGate.h"
#include <limits>
#include <stdlib.h>
#include <time.h>

using namespace std;

// Just for the parser
extern "C" int yyparse();
extern FILE *yyin; // Input file for parser

// Our circuit. We declare this external so the parser can use it more easily.
extern Circuit* myCircuit;

void printUsage();
vector<char> constructInputLine(string line);

//----------------------------
// If you add functions, please add the prototypes here.
void recurseSimulate(Gate*); //function created to obtain the outputs through each gate

//-----------------------------


int main(int argc, char* argv[]) {

  // Check the command line input and usage
  if (argc != 5) {
    printUsage();    
    return 1;
  }
  
  // Parse the bench file and initialize the circuit. (Using C style for our parser.)
  FILE *benchFile = fopen(argv[1], "r");
  if (benchFile == NULL) {
    cout << "ERROR: Cannot read file " << argv[1] << " for input" << endl;
    return 1;
  }
  yyin=benchFile;
  yyparse();
  fclose(benchFile);

  myCircuit->setupCircuit(); 
  cout << endl;

  // Setup the output text file
  ofstream outputStream;
  outputStream.open(argv[2]);
  if (!outputStream.is_open()) {
    cout << "ERROR: Cannot open file " << argv[2] << " for output" << endl;
    return 1;
  }
  
  ifstream faultStream;
  string faultLocStr;
  faultStream.open(argv[3]);
  if (!faultStream.is_open()) {
    cout << "ERROR: Cannot open fault file " << argv[3] << " for input" << endl;
    return 1;
  }
  
  // For each line in our fault file...
  while(getline(faultStream, faultLocStr)) {
      
    // Clear the old fault
    myCircuit->clearFaults();

    string faultTypeStr;
    // If we cannot get the second line in the fault pair, fail.
    if (!(getline(faultStream, faultTypeStr))) {
      break;
    }
      
    char faultType = atoi(faultTypeStr.c_str());

    // If == -1, then we are asking for fault-free simulation. 
    // Otherwise, set the fault in the circuit at the correct place.
    if (faultLocStr != "-1") {
      Gate* faultLocation = myCircuit->findGateByName(faultLocStr);      
      faultLocation->set_faultType(faultType);      
    }
    
    outputStream << "--" << endl;
    
    // Open the input vector file 
    ifstream inputStream;  
    string inputLine;  
    inputStream.open(argv[4]);
    if (!inputStream.is_open()) {
      cout << "ERROR: Cannot read file " << argv[4] << " for input" << endl;
      return 1;
    }
    
    
    // Try to read a line of inputs from the file.
    while(getline(inputStream, inputLine)) {
      
      // Clear logic values in my circuit and set new values
      // If there is a fault on the PI, the setPIValues() function
      // will automatically check if the value must be changed to D or D'.
      myCircuit->clearGateValues();
      myCircuit->setPIValues(constructInputLine(inputLine));
      
      /////////////////////////////////////////////////////////////////////////////
      // Write your code here.
      // At this point, the system has set up the data structure (like in Proj. 1) and 
      // set the input values for the PI gates (like in Proj. 1).
      // It has additionally used the new capability for placing faults on gate outputs
      // to set the fault in the appropriate gate.

	 vector<Gate*> primaryPOs = myCircuit->getPOGates(); 
	for (int j=0; j< primaryPOs.size(); j++)
	{
	recurseSimulate(primaryPOs[j]);	// calling the function to recusrively obtain the depth from back traversal taking one output gate at a time
	}



      
      
      // Stop writing your code here.
      ////////////////////////////////////////////////////
      
      // For each test vector, print the outputs and then the faults detectable at each gate.
      vector<Gate*> outputGates = myCircuit->getPOGates();
      for (int i=0; i < outputGates.size(); i++) {
		outputStream << outputGates[i]->printValue();
      }
      outputStream << endl;      
      
    }
    inputStream.close();
    
  }
  faultStream.close();
  
  // close the output and fault streams
  outputStream.close();
   
  return 0;
}

// Print usage information (if user provides incorrect input.
void printUsage() {
  cout << "Usage: ./faultsim [bench_file] [output_loc] [fault_file] [input_vectors]" << endl << endl;
  cout << "   bench_file:    the target circuit in .bench format" << endl;
  cout << "   output_loc:    location for output file" << endl;
  cout << "   fault_file:    faults to be simulated" << endl;
  cout << "   input_vectors: list of input vectors to simulate" << endl; 
  cout << endl;
  cout << "   Simulate each vector in input_vectors for each fault in fault_file." << endl;
  cout << "   For fault free simulation, enter -1 for the fault location and type in fault_file." << endl;
  cout << endl;
}

// Just used to parse in the values from the input file.
vector<char> constructInputLine(string line) {
  
  vector<char> inputVals;
  
  for (int i=0; i<line.size(); i++) {
    if (line[i] == '0') 
      inputVals.push_back(LOGIC_ZERO);
    
    else if (line[i] == '1') 
      inputVals.push_back(LOGIC_ONE);

    else if ((line[i] == 'X') || (line[i] == 'x')) {
      inputVals.push_back(LOGIC_X);
    }
   
    else {
      cout << "ERROR: Do not recognize character " << line[i] << " in input vector file." << endl;
      assert(false);
    }
  }  
  return inputVals;
}




////////////////////////////////////////////////////////////////////////////
// Place any new functions you add here, between these two bars.
//function to calculate the output of each gate starts

void recurseSimulate(Gate* g)
{
vector<Gate*> gate_inp=g->get_gateInputs();
for(int k=0 ; k< gate_inp.size(); k++)
{
	if(gate_inp[k]->getValue() == LOGIC_UNSET) //when the precessor gate output is unset, calling of the recursive function 
	{
		recurseSimulate(gate_inp[k]);
	}
}
	
	switch(g->get_gateType())
	{
		// calculation when the gate traversed is a BUFFER GATE
		case GATE_FANOUT://same as buffer		
		case GATE_BUFF:
				if (gate_inp[0]->getValue()==LOGIC_ZERO)
				{ if(g->get_faultType()==FAULT_SA1)
				g->setValue(LOGIC_DBAR);
				else 
				g->setValue(LOGIC_ZERO);				
				}
				else if (gate_inp[0]->getValue()==LOGIC_ONE)
				{ if(g->get_faultType()==FAULT_SA0)
				g->setValue(LOGIC_D);
				else
				g->setValue(LOGIC_ONE);	
				}
				else if( gate_inp[0]->getValue()==LOGIC_X)
				g->setValue(LOGIC_X);
				else if( gate_inp[0]->getValue()==LOGIC_D)
				g->setValue(LOGIC_D);
				else 
				g->setValue(LOGIC_DBAR);
				
				break;
		

/* all the AND,NAND,OR,NOR follows the below format

		for all inputs:
			is any input controlling value? then set respective output while checking faults
		
		if output still unset:
			for all inputs, is any value X? then set output X
		if output still unset:
			is any one input D:
				is any input DBAR:?then set respective output
				else set respective output
		if output still unset:
			is any input DBAR: set respective output
		if output still unset:
			remaining case is all non-controlling values as inputs:respective output 										while checking faults

Note: AND, and NAND adds one more condition to check if theres X and D and DBAR as input (exception case) */  


			// calculation when the gate traversed is a AND GATE
		case GATE_AND:	
			//checking if any zero input
			for(int j=0; j< gate_inp.size(); j++)
				{
				if(gate_inp[j]->getValue() == LOGIC_ZERO)
				{ if (g->get_faultType()==FAULT_SA1)
				{ g->setValue(LOGIC_DBAR);
				break; }
				else
				{g->setValue(LOGIC_ZERO);
				break;}
				}
				}
				//now check if any X input
			if (g->getValue() == LOGIC_UNSET) 
				{			
				for(int j=0; j< gate_inp.size(); j++)
				{		
				if (gate_inp[j]->getValue() == LOGIC_X)
				{ for (int l=0;l<gate_inp.size();l++)
				{//for starts here
				if (gate_inp[l]->getValue() == LOGIC_D)
				{ //if starts here
				for (int k=0;k<gate_inp.size();k++)
				{if (gate_inp[k]->getValue()==LOGIC_DBAR) 
				{g->setValue(LOGIC_ZERO); break;}
				}
				break;
				} // if ends here
										
				} //for ends here
				if (g->getValue() == LOGIC_UNSET)
				g->setValue(LOGIC_X);	
				} // if-check for X ends here
				}}				
				// now check if any input is D
			if(g->getValue() == LOGIC_UNSET)
				{//if starts here
				for(int j=0; j< gate_inp.size(); j++)
				{//for starts here
				if (gate_inp[j]->getValue() == LOGIC_D)
				{ //if starts here
					for (int l=0;l<gate_inp.size();l++)
					{
						if (gate_inp[l]->getValue() == LOGIC_DBAR)
						{ g->setValue(LOGIC_ZERO); break;}
					}
				if (g->getValue()==LOGIC_UNSET)
				{g->setValue(LOGIC_D);}
				break;
				}
				}//for ends here
				}//if ends here
			
				// now check if any input is DBAR
			if(g->getValue() == LOGIC_UNSET)
				{
				for(int j=0; j< gate_inp.size(); j++)
				{//for starts here
				if (gate_inp[j]->getValue() == LOGIC_DBAR)
				{ //if starts here
	           		 g->setValue(LOGIC_DBAR);
				 break; }
				}
				}
		
				// none of the above means, all 1's
				if(g->getValue() == LOGIC_UNSET)
				{
				if (g->get_faultType()==FAULT_SA0)
				g->setValue(LOGIC_D);
				else
				g->setValue(LOGIC_ONE);
			
				}
				break;
		

		// calculation when the gate traversed is a NAND GATE	
		case GATE_NAND:
		 	//checking if any zero input
			for(int j=0; j< gate_inp.size(); j++)
				{
				if(gate_inp[j]->getValue() == LOGIC_ZERO)
				{ if (g->get_faultType()==FAULT_SA0)
				{ g->setValue(LOGIC_D);
				break; }
				else
				{g->setValue(LOGIC_ONE);
				break;}
				}
				}// FOR ends here
			//now check if any X input
			if (g->getValue() == LOGIC_UNSET) 
				{			
				for(int j=0; j< gate_inp.size(); j++)
				{		
				if (gate_inp[j]->getValue() == LOGIC_X)
				{
				 for (int l=0;l<gate_inp.size();l++)
				{//for starts here
				if (gate_inp[l]->getValue()==LOGIC_D)
				{ //if starts here
					for (int k=0;k<gate_inp.size();k++)
					{
						if (gate_inp[k]->getValue()==LOGIC_DBAR) 
						{g->setValue(LOGIC_ONE); break;}
					}
					break;
				} // if ends here
										
				} //for ends here
				if (g->getValue() == LOGIC_UNSET)
					g->setValue(LOGIC_X);	
				} // if-check for X ends here
				}}				
			// now check if any input is D
			if(g->getValue() == LOGIC_UNSET)
				{//if starts here
				for(int j=0; j< gate_inp.size(); j++)
				{//for starts here
				if (gate_inp[j]->getValue() == LOGIC_D)
				{ //if starts here
				for (int l=0;l<gate_inp.size();l++)
				{//for starts here
				if (gate_inp[l]->getValue() == LOGIC_DBAR)
				{ g->setValue(LOGIC_ONE); break;}
				}//for ends here
				if (g->getValue()==LOGIC_UNSET)
				{g->setValue(LOGIC_DBAR);}
				break;
				}
				}//for ends here
				}//if ends here
			
			// now check if any input is DBAR
			if(g->getValue() == LOGIC_UNSET)
				{
				for(int j=0; j< gate_inp.size(); j++)
				{//for starts here
				if (gate_inp[j]->getValue() == LOGIC_DBAR)
				{ //if starts here
	           		 g->setValue(LOGIC_D);
				 break; }
				}
				}
		
			// none of the above means, all 1's
			if(g->getValue() == LOGIC_UNSET)
				{
				if (g->get_faultType()==FAULT_SA1)
				{ g->setValue(LOGIC_DBAR);
				}
	    			else
				{g->setValue(LOGIC_ZERO);
				}
				}
			break;
		
		
		// calculation when the gate traversed is a XOR GATE

		case GATE_XOR:
	
		if(gate_inp[0]->getValue()== LOGIC_X || gate_inp[1]->getValue()== LOGIC_X)
		g->setValue(LOGIC_X); //if atleast one input is X
	else if (gate_inp[0]->getValue()== gate_inp[1]->getValue()) //if same inputs
		{ if (g->get_faultType()==FAULT_SA1)
		g->setValue(LOGIC_DBAR);
		else
		g->setValue(LOGIC_ZERO);
		}
	else if (gate_inp[0]->getValue()== LOGIC_ONE || gate_inp[1]->getValue()== LOGIC_ONE) //if only one input is 1 (both inputs 1 case is already handled)and
		{ if (gate_inp[0]->getValue()== LOGIC_D || gate_inp[1]->getValue()== LOGIC_D)
				g->setValue(LOGIC_DBAR); // other input is D
	else if (gate_inp[0]->getValue()== LOGIC_DBAR || gate_inp[1]->getValue()== LOGIC_DBAR)//other input is DBAR
		g->setValue(LOGIC_D);
	else if (gate_inp[0]->getValue()== LOGIC_ZERO || gate_inp[1]->getValue()== LOGIC_ZERO)// other input is zero
		{ if (g->get_faultType()==FAULT_SA0)
		g->setValue(LOGIC_D);
		 else
		g->setValue(LOGIC_ONE);
		}
		}
	else if ((gate_inp[0]->getValue()== LOGIC_D && gate_inp[1]->getValue()== LOGIC_DBAR) || (gate_inp[0]->getValue()== LOGIC_DBAR && gate_inp[1]->getValue()== LOGIC_D))
				g->setValue(LOGIC_ONE);//If one input is D and other DBAR
		else //only one zero input case(both inputs zero case is already handled)
		{
		if (gate_inp[0]->getValue()== LOGIC_D || gate_inp[1]->getValue()== LOGIC_D)
		g->setValue(LOGIC_D);	
	else if (gate_inp[0]->getValue()== LOGIC_DBAR || gate_inp[1]->getValue()== LOGIC_DBAR)
		g->setValue(LOGIC_DBAR);
		}
								
		break;


		// calculation when the gate traversed is a XNOR GATE	
		case GATE_XNOR:
	
		if(gate_inp[0]->getValue()== LOGIC_X || gate_inp[1]->getValue()== LOGIC_X)
			g->setValue(LOGIC_X);//atleast one input as X case
	else if (gate_inp[0]->getValue()== gate_inp[1]->getValue())//if both inputs are same
		{ if (g->get_faultType()==FAULT_SA0)
		g->setValue(LOGIC_D);
		else
		g->setValue(LOGIC_ONE);
		}
	else if (gate_inp[0]->getValue()== LOGIC_ONE || gate_inp[1]->getValue()== LOGIC_ONE)//only one input as 1 (both inputs as 1 case is already handled) and
		{ 
		if (gate_inp[0]->getValue()== LOGIC_D || gate_inp[1]->getValue()== LOGIC_D)
		g->setValue(LOGIC_D);//other input is D
	else if (gate_inp[0]->getValue()== LOGIC_DBAR || gate_inp[1]->getValue()== LOGIC_DBAR)
		g->setValue(LOGIC_DBAR);//other input is DBAR
	else if (gate_inp[0]->getValue()== LOGIC_ZERO || gate_inp[1]->getValue()== LOGIC_ZERO)					//other input is 0
		{ if (g->get_faultType()==FAULT_SA1)
		g->setValue(LOGIC_DBAR);
		else
		g->setValue(LOGIC_ZERO);
		}
		}

	else if ((gate_inp[0]->getValue()== LOGIC_D && gate_inp[1]->getValue()== LOGIC_DBAR) || (gate_inp[0]->getValue()== LOGIC_DBAR && gate_inp[1]->getValue()== LOGIC_D))
				g->setValue(LOGIC_ZERO);//one input is D and other DBAR
		else		//only one input as 0(both inputs zero case already handled)
		{
		if (gate_inp[0]->getValue()== LOGIC_D || gate_inp[1]->getValue()== LOGIC_D)
				g->setValue(LOGIC_DBAR);	
	else if (gate_inp[0]->getValue()== LOGIC_DBAR || gate_inp[1]->getValue()== LOGIC_DBAR)
				g->setValue(LOGIC_D);
		}
								
		break;

		// calculation when the gate traversed is a OR GATE
		case GATE_OR:	
		//checking if any one input
		for(int j=0; j< gate_inp.size(); j++)
			{
			if(gate_inp[j]->getValue() == LOGIC_ONE)
			{ if (g->get_faultType()==FAULT_SA0)
			{ g->setValue(LOGIC_D);
			break; }
			else
			{g->setValue(LOGIC_ONE);
			break;}
			}
			}
		//now check if any X input
		if (g->getValue() == LOGIC_UNSET) 
			{			
			for(int j=0; j< gate_inp.size(); j++)
			{		
			if (gate_inp[j]->getValue() == LOGIC_X)
			{g->setValue(LOGIC_X); break;}
			}
			}				
		// now check if any input is D
		if(g->getValue() == LOGIC_UNSET)
			{//if starts here
			for(int j=0; j< gate_inp.size(); j++)
			{//for starts here
			if (gate_inp[j]->getValue() == LOGIC_D)
			{ //if starts here
				for (int l=0;l<gate_inp.size();l++)
				{
					if (gate_inp[l]->getValue() == LOGIC_DBAR)
					{ g->setValue(LOGIC_ONE); break;}
				}
				if (g->getValue()==LOGIC_UNSET)
				{g->setValue(LOGIC_D);}
				break;
			}//if ends here
			}//for ends here
			}//if ends here
			
		// now check if any input is DBAR
		if(g->getValue() == LOGIC_UNSET)
			{
			for(int j=0; j< gate_inp.size(); j++)
			{//for starts here
			if (gate_inp[j]->getValue() == LOGIC_DBAR)
			{ //if starts here
	           	 g->setValue(LOGIC_DBAR);
			break; }
			}
			}
		
		// none of the above means, all 0's
		if(g->getValue() == LOGIC_UNSET)
			{
			if (g->get_faultType()==FAULT_SA1)
			{ g->setValue(LOGIC_DBAR);
			}
	    		else
			{g->setValue(LOGIC_ZERO);
			}
			}
		break;
		
		// calculation when the gate traversed is a NOR GATE
		
	case GATE_NOR:

		//checking if any one input
		for(int j=0; j< gate_inp.size(); j++)
			{
			if(gate_inp[j]->getValue() == LOGIC_ONE)
			{ if (g->get_faultType()==FAULT_SA1)
			{ g->setValue(LOGIC_DBAR);
			break; }
			else
			{g->setValue(LOGIC_ZERO);
			break;}
			}
			}
		//now check if any X input
		if (g->getValue() == LOGIC_UNSET) 
			{			
			for(int j=0; j< gate_inp.size(); j++)
			{		
			if (gate_inp[j]->getValue() == LOGIC_X)
			{g->setValue(LOGIC_X); break;}
			}
			}				
		// now check if any input is D
		if(g->getValue() == LOGIC_UNSET)
			{//if starts here
			for(int j=0; j< gate_inp.size(); j++)
			{//for starts here
			if (gate_inp[j]->getValue() == LOGIC_D)
			{ //if starts here
				for (int l=0;l<gate_inp.size();l++)
				{
					if (gate_inp[l]->getValue() == LOGIC_DBAR)
					{ g->setValue(LOGIC_ZERO); break;}
				}
				if (g->getValue()==LOGIC_UNSET)
				{g->setValue(LOGIC_DBAR);}
			break;
			}//if ends here
			}//for ends here
			}//if ends here
			
		// now check if any input is DBAR
		if(g->getValue() == LOGIC_UNSET)
			{
			for(int j=0; j< gate_inp.size(); j++)
			{//for starts here
			if (gate_inp[j]->getValue() == LOGIC_DBAR)
			{ //if starts here
	           	 g->setValue(LOGIC_D);
			break; }
			}
			}
		
		// none of the above means, all 0's
		if(g->getValue() == LOGIC_UNSET)
			{
			if (g->get_faultType()==FAULT_SA0)
			{ g->setValue(LOGIC_D);
			}
	    		else
			{g->setValue(LOGIC_ONE);
			}
			}
		break;
	case GATE_NOT:	
		if(gate_inp[0]->getValue() == LOGIC_ONE)
			{ if (g->get_faultType()==FAULT_SA1)
			g->setValue(LOGIC_DBAR);
			else
			g->setValue(LOGIC_ZERO);
			}
		else if(gate_inp[0]->getValue() == LOGIC_X)
			g->setValue(LOGIC_X);
		else if (gate_inp[0]->getValue()==LOGIC_D)
			g->setValue(LOGIC_DBAR);
		else if (gate_inp[0]->getValue()==LOGIC_DBAR)
			g->setValue(LOGIC_D);
		else					
			{ if (g->get_faultType()==FAULT_SA0)
			g->setValue(LOGIC_D);
			 else
			g->setValue(LOGIC_ONE);	
			}			
				
			break;	
		
		
		 
			
	}

	
}
//function to calculate the output of each gate ends
////////////////////////////////////////////////////////////////////////////


