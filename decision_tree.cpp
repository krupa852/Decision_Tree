#include <unordered_map>
#include <set>
#include <list>
#include <algorithm>
#include<iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include<map>
#include <string>

class node;
using namespace std;
vector<string> Attributes;
vector<int> attribute_num;
int attribute_no =0;

//find binary attributes in data
class binary_attribute
{
	public:
		bool binary;
		map<string, int> val_count;
		void init()
    {
			binary = false;
		}
};

class target_info
{
  public:
	int count_true;//the number of True for target attribute for records with the value
	int count_false;//the number of False for target attribute for records with the value
	int count;//the total number of occurence for the value of the specified attribtue
  void init()
  {
		count =0;count_true =0;count_false =0;
	}
};

class child_node
{
  public:
	string branch;
	node *child;
};

class node
{
  public:
        string name;
        vector<child_node> children;
        void print() {
            cout << "name:" << this->name << "\nlen:" << this->name.length() << endl;
        }
};

vector<binary_attribute> attr_binary;
vector<vector<string>> Read_File(string file_name);
bool inputchoice_valid(vector<int> possible_choice_no,int choice_no);
node Decision_Tree(vector<vector<string>> datatable, int target_num, string target, vector<int> attribute_list,vector<string> attributes, vector<string> target_value);
string highest_count_targetattr_val(vector<vector<string>> datatable, int target_num);
int find_attribute_for_bestsplit(vector<vector<string>>  ,int target_num, vector<int> attribute_list,vector<string> target_value);
float count_before_entropy(vector<vector<string>> datatable, int target_num);
float count_after_entropy(vector<vector<string>> datatable, int attribute_num, int target_num,vector<string> target_value);
void split(vector<vector<string>> datatable,map<string, vector<int>> &split_datatable, int attribute_num,vector<child_node> &children);
void print_split_datatable(vector<vector<string>> datatable,map<string, vector<int>> split_datatable,node Node);
vector<vector<string>> form_datatable_by_index(vector<vector<string>> datatable, vector<int> index_vector);
void print_datatable(vector<vector<string>> new_datatable);
void Output_Tree(ofstream & output, node root, string target, vector<string>  target_value,int &num);
int delete_pos(int attribute_num,vector<int> attribute_list);

vector<vector<string>> Read_File(string file_name)
{
  ifstream infile(file_name);
  vector<vector<string>> attributevals;
	string attribute_names;
	string word;
	vector<vector<string>> info;
	int record_num =0;
	string line;

	/* code to extract features/Attribute names in file */
	getline(infile, attribute_names);
	istringstream ss(attribute_names);
  	while( ss >> word)
	{

		Attributes.push_back(word);
		attribute_num.push_back(attribute_no++);
  };

	string Class_Label = Attributes[Attributes.size()-1];

/*	for(int i=0;i<Attributes.size();i++)
		cout<<Attributes[i]<<endl;

	for(int i=0;i<attribute_num.size();i++)
                cout<<attribute_num[i]<<endl;*/
	for(int i =0; i < attribute_no; i++)
	{
		binary_attribute temp;
		attr_binary.push_back(temp);
	}

	/*code to extract attribute values from file */
	while(getline(infile, line))//read file line by line
  	{
                istringstream iss(line);
		int iterate_attribute =0;
                vector<string> tokens;
                copy(istream_iterator<string>(iss),
                        istream_iterator<string>(),
                        back_inserter(tokens));
                vector<string> attributeval;
                for(int i=0;i<tokens.size();i++)
                {
			if(attr_binary[iterate_attribute].val_count.find(tokens[i]) == attr_binary[iterate_attribute].val_count.end()){//not found
				attr_binary[iterate_attribute].val_count[tokens[i]] = 1;
			}else{
				attr_binary[iterate_attribute].val_count[tokens[i]]++;
			}
			iterate_attribute++;
                        attributeval.push_back((tokens[i]));
                }
                attributevals.push_back(attributeval);
        }

	//get binary attributes
        for(int i = 0; i < attribute_no; i++){
                if(attr_binary[i].val_count.size()==2)
		{
                        attr_binary[i].binary = true;
                }	
		else
		{
                        attr_binary[i].binary = false;
                }

        }

	//print datatable
      /*for(int i=0;i<attributevals.size();i++)
        {
                for(int j=0;j<attributevals[i].size();j++)
                {cout<<attributevals[i][j];}
		cout<<endl;
        }*/

        return attributevals;
}

bool inputchoice_valid(vector<int> possible_choice_no,int choice_no){
	for(auto i = possible_choice_no.begin(); i!=possible_choice_no.end();i++){
		if((*i)==choice_no){
			return true;
		}
	}
	return false;
}

//check whether all records in the datatable belongs to one catergory or not
bool check_label(vector<vector<string>> datatable,int target_num, string &name)
{
        for(auto i = datatable.begin(); i != datatable.end();i++)
	{
                if(i == datatable.begin())
		{
                        name = (*i)[target_num];
		}
                string temp = (*i)[target_num];
                if(name!=temp)
		{
                        return false;
                }
	}
        return true;
}

node Decision_Tree(vector<vector<string>> datatable, int target_num,string target, vector<int> attribute_list,vector<string> attributes, vector<string> target_value){
	node *Node;
  	Node = new node;
	string name;
	if(check_label(datatable,target_num,name))
	{	//all records have the same label
		Node->name = name;
	}
	else if(attribute_list.size() == 0)
	{	//the attribute list is empty
		Node->name = highest_count_targetattr_val(datatable,target_num);
	}
	else
	{	// determine the attribute test condition
		//get attribute which gets best split
		int attribute_num = find_attribute_for_bestsplit(datatable,target_num,attribute_list, target_value);
                Node->name = attributes[attribute_num];

		//split the datatable based on the determined attribute and name new branch.
		map<string, vector<int>> split_datatable;//first represents the value of the attribute. second represents the index of records in original datatable with the value for the attribute
		split(datatable,split_datatable,attribute_num,Node->children);
		//	print_split_datatable(datatable, split_datatable,*Node);

		//remove the used attribute from attribute list and attributes
		int position = delete_pos(attribute_num,attribute_list);
		attribute_list.erase(attribute_list.begin()+ position);

		//test stopping point for each splitted datatable
		for(auto v: split_datatable)
		{//for each splitted datatable
			//find the child_node
			auto k =Node->children.begin();
			for(auto j =Node->children.begin(); j != Node->children.end(); j++)
			{
				if((*j).branch == v.first)
				{
					k = j;
					break;
				}
			}
			//form new datatable using index
			vector<vector<string>> new_datatable = form_datatable_by_index(datatable, v.second);
			//print_datatable(new_datatable);
			node *child_Node;
			child_Node = new node;
			string name;

			if(check_label(new_datatable,target_num,name))
			{//all records in the current splitted databases belong to the same label
				child_Node->name = name;
				(*k).child = child_Node;
			}
			else if(attribute_list.size()==0)
			{//attribute list is empty
				child_Node->name = highest_count_targetattr_val(new_datatable, target_num);
				(*k).child = child_Node;

			}
			else if(new_datatable.size()==0)
			{//no sample
				child_Node->name = "no sample";
				(*k).child = child_Node;
			}
			else
			{//recursively call Decision_Tree
				(*child_Node) = Decision_Tree(new_datatable, target_num,target,attribute_list, attributes, target_value);
				(*k).child = child_Node;
			}
		}
	}

	return *Node;
}

//get the position of the element attribute_num in a vector
int delete_pos(int attribute_num,vector<int> attribute_list)
{
	int position = -1;
	for(auto i = attribute_list.begin(); i!=attribute_list.end();i++)
	{
		position++;
		if((*i)==attribute_num){
			return position;
		}
	}
	cout <<"failed to find the position of the attribute to be deleted"<<endl;
	return -1;
}


//form datatable using index
vector<vector<string>> form_datatable_by_index(vector<vector<string>> datatable, vector<int> index_vector)
{
	vector<vector<string>> new_datatable;
	for(auto i = index_vector.begin(); i != index_vector.end(); i++)
	{
		auto j = datatable.begin()+(*i);
		vector<string> record;
		for(auto m=(*j).begin(); m != (*j).end(); m++)
		{
			record.push_back(*m);
		}
		new_datatable.push_back(record);
	}
	return new_datatable;
}


//split datatable based on the determined attribute and store the newly splitted datatable in the form of index of the original datatable and name branch.
void split(vector<vector<string>> datatable,map<string, vector<int>> &split_datatable, int attribute_num, vector<child_node> &children)
{
	int index= 0;
	for(auto i = datatable.begin(); i != datatable.end(); i++)
	{	//scan original datatable
		if(split_datatable.find((*i)[attribute_num])==split_datatable.end())
		{	//the value of the attribute has not been found yet
			child_node child_Node_ref;
			child_Node_ref.branch = (*i)[attribute_num];//name the branch
			children.push_back(child_Node_ref);
		}
		split_datatable[(*i)[attribute_num]].push_back(index);//save records to corresponding splitted datatable
		index++;
	}
}


//for testing the splitted datatables
void print_split_datatable(vector<vector<string>> datatable,map<string, vector<int>> split_datatable, node Node)
{
	for(auto v: split_datatable)
	{
		cout << "datatable "<< v.first<<":"<< endl;
		for(auto i = v.second.begin(); i != v.second.end(); i++)
		{
			for(int j =0; j< 4;j++)
			{
				cout << datatable[*i][j]<< " ";
			}
			cout << endl;
		}
	}
}


//for testing datatable
void print_datatable(vector<vector<string>> new_datatable)
{
	cout << "print datatable:"<<endl;
	for(auto i = new_datatable.begin(); i != new_datatable.end(); i++)
	{
		for(auto j = (*i).begin(); j != (*i).end(); j++){
			cout << (*j)<<" ";
		}
		cout << endl;
	}
}


//get the name of the highest count targetattribute value
string highest_count_targetattr_val(vector<vector<string>> datatable, int target_num)
{
	//records the occurence of different attribute's values
	map<string,int> values;
	for(auto i = datatable.begin(); i != datatable.end();i++)
	{
		if(values.find((*i)[target_num])==values.end())
		{	//the value is not found
			values[(*i)[target_num]] = 1;
		}else
		{
			values[(*i)[target_num]]++;
		}
	}
	//get the value which occurs most frequently
	int count =0 ;
	string high_cnt_val;
	for(auto v:values)
	{
		if(v.second> count)
		{
			count = v.second;
			high_cnt_val = v.first;
		}
	}
	return high_cnt_val;
}

//get attribute which gets best split
int find_attribute_for_bestsplit(vector<vector<string>> datatable,int target_num, vector<int> attribute_list, vector<string> target_value){
	map<int, float> info_gain;//store information gain for all attributes of attribute list
	float before_entropy = count_before_entropy(datatable,target_num);
	float after_entropy = 0;
	for(auto i = attribute_list.begin(); i != attribute_list.end(); i++)
	{
		//count and save information gain for each of the attribute in attribute list
		after_entropy = count_after_entropy(datatable, (*i), target_num, target_value);
		info_gain[(*i)] = before_entropy-after_entropy;
	}
	//find the best split attribute by the highest information gain
	int best_split_attribute_num=-1;
	float max_info_gain =0;
	for(auto v: info_gain){
		if(v.second>=max_info_gain)
		{
			max_info_gain = v.second;
			best_split_attribute_num=v.first;
		}
	}
	if(best_split_attribute_num==-1)
	{
		cout << "not found best split"<<endl;
		if(attribute_list.size()==0)
			cout << "attribute_list is empty"<<endl;
	}
	return  best_split_attribute_num;
}

//calculate entropy before splitting
float count_before_entropy(vector<vector<string>> datatable, int target_num)
{
	float bef_entropy_value = 0.0;
	map<string, int> before;//first:value for target,second:count its occurence
	int count=0;//store the number of records in the datatable
	for(auto i = datatable.begin(); i != datatable.end(); i++)
	{//calculate the occurence of possible value for target attribute
		if(before.find((*i)[target_num])==before.end())
		{
			before[(*i)[target_num]] =1;
		}
		else
		{
			before[(*i)[target_num]]++;
		}
		count++;
	}
/*	for (auto itr = before.begin(); itr != before.end(); itr++)
        cout << itr->first
             << '\t' << itr->second << '\n';*/
	//calculate the probability of all possible value of target attribute
	for(auto v: before)
	{
		float probability = float(v.second)/float(count);
		if(probability == 0)
		{
			;
		}
		else
		{
			bef_entropy_value += -(probability)*log2(probability);
		}
	}
	return bef_entropy_value;
}

float count_after_entropy(vector<vector<string>> datatable, int attribute_num, int target_num, vector<string> target_value)
{
	//get target_info on the information for different values of the specified attribute
	int count = 0;//the number of records in the current datatable
	map<string,target_info> spec_attr_info;//first:the value of the specified attribute,second: target_info on the value of the attribute
	for(auto i = datatable.begin(); i != datatable.end(); i++)
{
		if(spec_attr_info.find((*i)[attribute_num])==spec_attr_info.end())
		{//not found yet
			spec_attr_info[(*i)[attribute_num]].count = 1;
			if((*i)[target_num]==target_value[0])
			{
				spec_attr_info[(*i)[attribute_num]].count_true =1;
			}
			else
			{
				spec_attr_info[(*i)[attribute_num]].count_false = 1;
			}
		}
		else
		{
			spec_attr_info[(*i)[attribute_num]].count++;
			if((*i)[target_num]==target_value[0])
			{
                                spec_attr_info[(*i)[attribute_num]].count_true++;
                        }else
			{
                                spec_attr_info[(*i)[attribute_num]].count_false++;
                        }
		}
			count++;
	}

	//calculate the entropy based on the collected target_info
	float entropy =0;
	for(auto v:spec_attr_info){
		float probability_of_attribute_value =float(v.second.count)/float(count);
		float probability_of_targetval_T = float(v.second.count_true)/float(v.second.count);
		float probability_of_targetval_F = float(v.second.count_false)/float(v.second.count);
		if(probability_of_targetval_T ==0 ||probability_of_targetval_F == 0){
			entropy = 0;
		}else
		{
			entropy +=(-probability_of_attribute_value)*(probability_of_targetval_T*log2(probability_of_targetval_T)+probability_of_targetval_F*log2(probability_of_targetval_F));
		}
	}
	return entropy;
}

//print the tree in a file.
void Output_Tree(ofstream & output, node root, string target, vector<string>  target_value, int &num)
{
	if(root.children.size()==0)
	{//end node
		output<< target << " is "<< root.name<<"."<<endl;
	}
	else
	{
		output << endl;
		for(auto a = root.children.begin(); a != root.children.end(); a++)
		{
			int temp = num;
			int k =num;
			while(temp)
			{
				output << "	";
				temp--;
			}
			output << "if "<< root.name<< " is "<< (*a).branch<<", then"<<" ";
			num++;
			Output_Tree(output, *((*a).child),target, target_value,num);
			num = k;
		}
	}
}

int main(int argc, char* argv[])
{

	vector<vector<string> > Train_data;
        // parse the input arguments
        if(argc<2)
        {
                fprintf(stderr,"incorrect number of arguments, terminating the program...\n");
                return 0;
        }
        string train_file_name = string(argv[1]);
        Train_data = Read_File(train_file_name);
	//choose target attribute
	cout << "Please choose an attribute (by number) as the prediction attribute: " << endl;
	map<int, int> target_map;
	int choice_num=0;
	vector<int> possible_choice_no;
	for(int i =0; i < attribute_no;i++)
	{
		if(attr_binary[i].binary ==true)
		{
			choice_num++;
			cout << choice_num<<"."<< Attributes[i]<<endl;
			possible_choice_no.push_back(choice_num);
			target_map[choice_num] = i;
		}
	}
	int choice_no;
	cout << "your choice:"<<endl;
	cin >> choice_no;

	//check whether choice is valid
	bool valid;
	valid = inputchoice_valid(possible_choice_no,choice_no);
	if(valid==false)
	{
		cout <<"choice is invalid."<<endl;
		return 0;
	}

	//get related parameters for Decision_Tree
	//the target related info from choice_num
	int target_num = target_map[choice_no];
	string target = Attributes[target_num];

	//get the attribute list
	attribute_num.erase(attribute_num.begin() + target_num);

	//get possible target value
	vector<string> target_value;
	for(auto v:attr_binary[target_num].val_count)
	target_value.push_back(v.first);

	node root = Decision_Tree(Train_data,target_num,target,attribute_num,Attributes,target_value);

	ofstream output;
  	output.open("Tree.txt");
	int num =0;
	Output_Tree(output,root,target, target_value,num);
	output.close();
	return 0;

}
