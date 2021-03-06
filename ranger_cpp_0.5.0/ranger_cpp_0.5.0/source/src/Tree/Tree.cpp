/*-------------------------------------------------------------------------------
 This file is part of Ranger.

 Ranger is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Ranger is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Ranger. If not, see <http://www.gnu.org/licenses/>.

 Written by:

 Marvin N. Wright
 Institut für Medizinische Biometrie und Statistik
 Universität zu Lübeck
 Ratzeburger Allee 160
 23562 Lübeck

 http://www.imbs-luebeck.de
 wright@imbs.uni-luebeck.de
 #-------------------------------------------------------------------------------*/

#include <iterator>
#include <queue>
#include "Tree.h"
#include "utility.h"
#include <stdio.h>
Tree::Tree() :
    dependent_varID(0), mtry(0), num_samples(0), num_samples_oob(0), is_ordered_variable(0), no_split_variables(0), min_node_size(
        0), deterministic_varIDs(0), split_select_varIDs(0), split_select_weights(0), case_weights(0), oob_sampleIDs(0), holdout(
        false), keep_inbag(false), data(0), validation_data(0), test_data(0), variable_importance(0), importance_mode(DEFAULT_IMPORTANCE_MODE), sample_with_replacement(
        true), sample_fraction(1), memory_saving_splitting(false), splitrule(DEFAULT_SPLITRULE), alpha(DEFAULT_ALPHA), minprop(
        DEFAULT_MINPROP) {
}

Tree::Tree(std::vector<std::vector<size_t>>& child_nodeIDs, std::vector<size_t>& split_varIDs,
    std::vector<double>& split_values, std::vector<bool>* is_ordered_variable) :
    dependent_varID(0), mtry(0), num_samples(0), num_samples_oob(0), is_ordered_variable(is_ordered_variable), no_split_variables(
        0), min_node_size(0), deterministic_varIDs(0), split_select_varIDs(0), split_select_weights(0), case_weights(0), split_varIDs(
        split_varIDs), split_values(split_values), child_nodeIDs(child_nodeIDs), oob_sampleIDs(0), holdout(false), keep_inbag(
        false), data(0), validation_data(0), test_data(0), variable_importance(0), importance_mode(DEFAULT_IMPORTANCE_MODE), sample_with_replacement(
        true), sample_fraction(1), memory_saving_splitting(false), splitrule(DEFAULT_SPLITRULE), alpha(DEFAULT_ALPHA), minprop(
        DEFAULT_MINPROP) {
}

Tree::~Tree() {
}

void Tree::init(Data* data, Data* validation_data, Data* test_data, uint mtry, size_t dependent_varID, size_t num_samples, uint seed,
    std::vector<size_t>* deterministic_varIDs, std::vector<size_t>* split_select_varIDs,
    std::vector<double>* split_select_weights, ImportanceMode importance_mode, uint min_node_size,
    std::vector<size_t>* no_split_variables, bool sample_with_replacement, std::vector<bool>* is_unordered,
    bool memory_saving_splitting, SplitRule splitrule, std::vector<double>* case_weights, bool keep_inbag,
    double sample_fraction, double alpha, double minprop, bool holdout, uint prepruning,uint postpruning) {

  this->data = data;
  this->validation_data = validation_data;
  this->test_data = test_data;
  this->mtry = mtry;
  this->dependent_varID = dependent_varID;
  this->num_samples = num_samples;
  this->memory_saving_splitting = memory_saving_splitting;
  
  //pruning
  this->prepruning = prepruning;
  this->postpruning = postpruning;
  // Create root node, assign bootstrap sample and oob samples
  child_nodeIDs.push_back(std::vector<size_t>());
  child_nodeIDs.push_back(std::vector<size_t>());
  createEmptyNode();

  // Initialize random number generator and set seed
  random_number_generator.seed(seed);

  this->deterministic_varIDs = deterministic_varIDs;
  this->split_select_varIDs = split_select_varIDs;
  this->split_select_weights = split_select_weights;
  this->split_select_weights = split_select_weights;
  this->importance_mode = importance_mode;
  this->min_node_size = min_node_size;
  this->no_split_variables = no_split_variables;
  this->is_ordered_variable = is_unordered;
  this->sample_with_replacement = sample_with_replacement;
  this->splitrule = splitrule;
  this->case_weights = case_weights;
  this->keep_inbag = keep_inbag;
  this->sample_fraction = sample_fraction;
  this->holdout = holdout;
  this->alpha = alpha;
  this->minprop = minprop;

  initInternal();
}

/*bool Tree::check_pruning(size_t nodeID){
         return true;
}*/

void Tree::postPruning(uint postpruningNumber){
   size_t ID = 0;
   size_t terminal = node.size()-2;
   size_t left_child_ID;
   size_t right_child_ID;
   size_t tree_hight = node.size();
  for(int i =tree_hight ; i > 1 ; i--) {
      for(int j = 1; j < node[i-2].size() ; j++ ) {
        
       		 ID = node[i-2][j];
              if(postpruningNumber == 1) {
                if(REPpruning(ID))
                  merge(ID);
              }
      }
  }

}

bool Tree::REPpruning(size_t ID){
     bool prune = false;
     if(child_nodeIDs[0][ID] != 0 && child_nodeIDs[1][ID] != 0) {
        prune = errorCompare(ID);
     }
     return prune;
}

void Tree::traceLeafNode(size_t nodeID) {
  while(true) {
      //Break if terminal node
      if(child_nodeIDs[0][nodeID] == 0 && child_nodeIDs[1][nodeID] == 0){
          break;
      }
      if(sampleIDs[child_nodeIDs[0][nodeID]].size() >= sampleIDs[child_nodeIDs[1][nodeID]].size())
        nodeID = child_nodeIDs[0][nodeID];
      else nodeID = child_nodeIDs[1][nodeID];
  }
  split_values.push_back(split_values[nodeID]);  
  split_varIDs.push_back(0);
    
}
bool Tree::errorCompare(size_t ID){
  size_t LR_node = -1;//node ID is left node or right node
  size_t origin_leftNode = child_nodeIDs[0][ID];
  size_t origin_RightNode = child_nodeIDs[1][ID];
  size_t current_ID = child_nodeIDs[0].size();//current_ID = node number +1

  if(child_nodeIDs[0][parent_node[ID]] == ID){
    LR_node = 0;
    child_nodeIDs[0][parent_node[ID]] = current_ID;
  }
  else if(child_nodeIDs[1][parent_node[ID]] == ID){
    LR_node = 1;
    child_nodeIDs[1][parent_node[ID]] = current_ID;
  }
  else return false;
  
  child_nodeIDs[0].push_back(0);
  child_nodeIDs[1].push_back(0);
  traceLeafNode(ID);
 
  after_Pruning = errorOfTree();
  //restore origin node
  child_nodeIDs[0].pop_back();
  child_nodeIDs[1].pop_back();
  split_varIDs.pop_back();
  split_values.pop_back();
  if(LR_node == 0)
    child_nodeIDs[0][parent_node[ID]] = ID;
  else if(LR_node == 1)
    child_nodeIDs[1][parent_node[ID]] = ID;
  //if error of after Pruning < error of before pruning, -->pruning
  if(after_Pruning < before_Pruning)
    return true;
  else return false;
}
//add
int Tree::errorOfTree(){
  size_t error = 0;
  size_t number_samples_validation = validation_data->getNumRows();
  size_t ID = 0;
  size_t sample_index = 0;
  while(sample_index <= number_samples_validation){
    ID = 0;
    while(true){
      //Break if terminal node
      if(child_nodeIDs[0][ID] == 0 && child_nodeIDs[1][ID] == 0){
        break;
      }
      //split attribute ID
      size_t split_varID = split_varIDs[ID];

      double val = validation_data->get(sample_index, split_varID);
      //is_ordered_variable
      if ((*is_ordered_variable)[split_varID]) {
        if (val <= split_values[ID]) {
          // Move to left child
          ID = child_nodeIDs[0][ID];
        } else {
          // Move to right child
          ID = child_nodeIDs[1][ID];
        }
      } else {
          size_t factorID = floor(val) - 1;
          size_t splitID = floor(split_values[ID]);

          // Left if 0 found at position factorID
          if (!(splitID & (1 << factorID))) {
            // Move to left child
            ID = child_nodeIDs[0][ID];
          } else {
            // Move to right child
            ID = child_nodeIDs[1][ID];
          }
      }
    }
    double real_value = validation_data->get(sample_index, dependent_varID);
    if(split_values[ID] != real_value){
      error++;
    }
    sample_index++;
  }
  //return error
  return error;
}

void Tree::merge(size_t nodeID){
   
   if(!isLeafNode(child_nodeIDs[0][nodeID])){
      merge(child_nodeIDs[0][nodeID]);  
   }
   if(!isLeafNode(child_nodeIDs[1][nodeID])){
      merge(child_nodeIDs[1][nodeID]);
   }  
   if(isLeafNode(child_nodeIDs[0][nodeID])&&isLeafNode(child_nodeIDs[1][nodeID])){
      size_t left_child_ID = child_nodeIDs[0][nodeID];
      size_t right_child_ID = child_nodeIDs[1][nodeID];
      if(sampleIDs[left_child_ID].size()> sampleIDs[right_child_ID].size()) {
          split_values[nodeID] = split_values[left_child_ID];
          split_varIDs[nodeID] = 0; 
      }
      else {
          split_values[nodeID] = split_values[right_child_ID];
          split_varIDs[nodeID] = 0;
      } 
       deleteLeafNode(child_nodeIDs[0][nodeID]);
       deleteLeafNode(child_nodeIDs[1][nodeID]);
       leaf_node.push_back(nodeID); 
       
      node_size--;//add
      sampleIDs[child_nodeIDs[0][nodeID]].resize(0);//add
      sampleIDs[child_nodeIDs[1][nodeID]].resize(0);  //add
      child_nodeIDs[0][nodeID] = 0;
      child_nodeIDs[1][nodeID] = 0;  
      before_Pruning = after_Pruning;  //add 
   
   }
 
}

bool Tree::isLeafNode(size_t nodeID){
  
  for(int i = 0;i<leaf_node.size();i++){
      
       if(nodeID == leaf_node[i])
         return true;
  }
   
  return false;
}

void Tree::deleteLeafNode(size_t nodeID){
     
     for(int i =0;i<leaf_node.size();i++){
                
            if(nodeID == leaf_node[i]){
                leaf_node.erase(leaf_node.begin()+i);
                break;
             }
     }
   
}

void Tree::record_trigger_node(size_t nodeID){

     if(sampleIDs[nodeID].size() <= min_node_size){
            trigger_node.push_back(nodeID);
     }
     else{

            if(child_nodeIDs[0][nodeID] != 0)
               record_trigger_node(child_nodeIDs[0][nodeID]);
            if (child_nodeIDs[1][nodeID] != 0)
               record_trigger_node(child_nodeIDs[1][nodeID]);
     }


}

void Tree::grow(std::vector<double>* variable_importance) {

  this->variable_importance = variable_importance;

// Bootstrap, dependent if weighted or not and with or without replacement
  if (case_weights->empty()) {
    if (sample_with_replacement) {
      bootstrap();
    } else {
      bootstrapWithoutReplacement();
    }
  } else {
    if (sample_with_replacement) {
      bootstrapWeighted();
    } else {
      bootstrapWithoutReplacementWeighted();
    }
  }

// While not all nodes terminal, split next node
  size_t num_open_nodes = 1;
  size_t i = 0;
  size_t level = 0;
  size_t ID = 0;
  parent_node.push_back(0);
  node_light = 0;
  node_normal =0; 
  while (num_open_nodes > 0) {
    bool is_terminal_node = splitNode(i);
    if (is_terminal_node) {
      --num_open_nodes;
    } else {
      ++num_open_nodes;
    }
    ++i;
  }
  
  temp.push(0);
  node.push_back(std::vector<size_t>());
  node[0].push_back(1);
  node[0].push_back(0);
  
  while(!temp.empty()){
    ++level;
    node.push_back(std::vector<size_t>());
    node[level].push_back(0);
   for(int i = 0, n = temp.size(); i < n; i++){
   	 ID = temp.front();
         temp.pop();
         if(child_nodeIDs[0][ID] != 0){
           temp.push(child_nodeIDs[0][ID]);
           ++node[level][0];
           node[level].push_back(child_nodeIDs[0][ID]);
         }
         if(child_nodeIDs[1][ID] != 0){
           temp.push(child_nodeIDs[1][ID]);
           ++node[level][0];
           node[level].push_back(child_nodeIDs[1][ID]);
         }
   }
    
  }
 size_t node_count = 0;
 for(int i =0 ; i< node.size() ; i++){
     node_count+=node[i][0];
 } 
 printf("total node %d\n",node_count);
 printf("normal %d\n",node_normal);
 printf("light %d\n",node_light);
 //add
 std::cout<<"Postpruning-------------------------------------"<<std::endl;
  before_Pruning = errorOfTree();
  double B_node_size = sampleIDs.size();
  std::cout<<std::endl<<"error before pruning: "<< before_Pruning <<std::endl;
  std::cout<<"error rate: "<< (before_Pruning/validation_data->getNumRows())*100<< "%" <<std::endl;
  std::cout<<"number of node before pruning: "<< B_node_size <<std::endl<<std::endl;
  double A_node_size;
  node_size = B_node_size;
  //postpruning
  if(postpruning != 0){
    std::cout<<"executing postpruning ..."<<std::endl<<std::endl;
    time_t t_start = time(NULL);
    postPruning(postpruning);
    time_t t_end = time(NULL);
    std::cout<<"pruning time:"<< t_end - t_start <<std::endl;
    A_node_size = node_size;
    if(before_Pruning < after_Pruning)
        after_Pruning = before_Pruning;
    std::cout<<"error after pruning: "<< after_Pruning <<std::endl;
    std::cout<<"error rate: "<< (after_Pruning/validation_data->getNumRows())*100 << "%" <<std::endl;
    std::cout<<"number of node after pruning: "<< A_node_size <<std::endl<<std::endl;
 }
 std::cout<<"reduce node: "<< B_node_size - A_node_size <<std::endl;
 
 std::cout<<"Prepruning---------------------------------------"<<std::endl;
 //prepruning
 if(prepruning != 0){
     record_trigger_node(0);  
    
    //Original accuracy
     predict(test_data,false);
     computeAccuracy();
     printf("Original accuracy  : %lf\n",tree_accuracy);
     //pruning
    for(int i =0 ; i<trigger_node.size(); i++) {
          if(!isLeafNode(trigger_node[i])) {
             
               merge(trigger_node[i]);
          }
    }
    std::cout<<min_node_size<<std::endl;
    std::cout<<trigger_node.size()<<std::endl; 
    //Prepruning accuracy
     predict(test_data,false);
     computeAccuracy();
     printf("Prepruned accuracy : %lf\n",tree_accuracy);
     std::cout<<leaf_node.size()<<std::endl;
 }

// Delete sampleID vector to save memory
  sampleIDs.clear();
  cleanUpInternal();

}

void Tree::computeAccuracy() {
  
  size_t num_missclassifications = 0; 
  for(int i = 0 ; i<prediction_terminal_nodeIDs.size(); i++) {
      size_t nodeID = prediction_terminal_nodeIDs[i];
      double predicted_value  = split_values[nodeID]; 
      double real_value = test_data->get(i,dependent_varID);
      if(predicted_value != real_value) {
        ++num_missclassifications;
      } 
  }     
  
  tree_accuracy =(double)1-( (double)num_missclassifications / (double)prediction_terminal_nodeIDs.size());
  
}

void Tree::predict(const Data* prediction_data, bool oob_prediction) {

  size_t num_samples_predict;
  if (oob_prediction) {
    num_samples_predict = num_samples_oob;
  } else {
    num_samples_predict = prediction_data->getNumRows();
  }

  prediction_terminal_nodeIDs.resize(num_samples_predict, 0);

// For each sample start in root, drop down the tree and return final value
  for (size_t i = 0; i < num_samples_predict; ++i) {
    size_t sample_idx;
    if (oob_prediction) {
      sample_idx = oob_sampleIDs[i];
    } else {
      sample_idx = i;
    }
    size_t nodeID = 0;
    while (1) {

      // Break if terminal node
      if (child_nodeIDs[0][nodeID] == 0 && child_nodeIDs[1][nodeID] == 0) {
        break;
      }

      // Move to child
      size_t split_varID = split_varIDs[nodeID];
      double value = prediction_data->get(sample_idx, split_varID);
      if ((*is_ordered_variable)[split_varID]) {
        if (value <= split_values[nodeID]) {
          // Move to left child
          nodeID = child_nodeIDs[0][nodeID];
        } else {
          // Move to right child
          nodeID = child_nodeIDs[1][nodeID];
        }
      } else {
        size_t factorID = floor(value) - 1;
        size_t splitID = floor(split_values[nodeID]);

        // Left if 0 found at position factorID
        if (!(splitID & (1 << factorID))) {
          // Move to left child
          nodeID = child_nodeIDs[0][nodeID];
        } else {
          // Move to right child
          nodeID = child_nodeIDs[1][nodeID];
        }
      }
    }

    prediction_terminal_nodeIDs[i] = nodeID;
  }
}

void Tree::computePermutationImportance(std::vector<double>* forest_importance, std::vector<double>* forest_variance) {

  size_t num_independent_variables = data->getNumCols() - no_split_variables->size();

// Compute normal prediction accuracy for each tree. Predictions already computed..
  double accuracy_normal = computePredictionAccuracyInternal();

  prediction_terminal_nodeIDs.clear();
  prediction_terminal_nodeIDs.resize(num_samples_oob, 0);

// Reserve space for permutations, initialize with oob_sampleIDs
  std::vector<size_t> permutations(oob_sampleIDs);

// Randomly permute for all independent variables
  for (size_t i = 0; i < num_independent_variables; ++i) {

    // Skip no split variables
    size_t varID = i;
    for (auto& skip : *no_split_variables) {
      if (varID >= skip) {
        ++varID;
      }
    }

    // Permute and compute prediction accuracy again for this permutation and save difference
    permuteAndPredictOobSamples(varID, permutations);
    double accuracy_permuted = computePredictionAccuracyInternal();
    double accuracy_difference = accuracy_normal - accuracy_permuted;
    (*forest_importance)[i] += accuracy_difference;

    // Compute variance
    if (importance_mode == IMP_PERM_BREIMAN) {
      (*forest_variance)[i] += accuracy_difference * accuracy_difference;
    } else if (importance_mode == IMP_PERM_LIAW) {
      (*forest_variance)[i] += accuracy_difference * accuracy_difference * num_samples_oob;
    }
  }
}

void Tree::appendToFile(std::ofstream& file) {

// Save general fields
  saveVector2D(child_nodeIDs, file);
  saveVector1D(split_varIDs, file);
  saveVector1D(split_values, file);

// Call special functions for subclasses to save special fields.
  appendToFileInternal(file);
}

void Tree::createPossibleSplitVarSubset(std::vector<size_t>& result) {

// Always use deterministic variables
  std::copy(deterministic_varIDs->begin(), deterministic_varIDs->end(), std::inserter(result, result.end()));

// Randomly add non-deterministic variables (according to weights if needed)
  if (split_select_weights->empty()) {
    drawWithoutReplacementSkip(result, random_number_generator, data->getNumCols(), *no_split_variables, mtry);
  } else {
    size_t num_draws = mtry - result.size();
    drawWithoutReplacementWeighted(result, random_number_generator, *split_select_varIDs, num_draws,
        *split_select_weights);
  }
}

bool Tree::splitNode(size_t nodeID) {

// Select random subset of variables to possibly split at
  std::vector<size_t> possible_split_varIDs;
  createPossibleSplitVarSubset(possible_split_varIDs);
  
  
// Call subclass method, sets split_varIDs and split_values
  bool stop = splitNodeInternal(nodeID, possible_split_varIDs,prepruning);
  if (stop) {
    // Terminal node
    leaf_node.push_back(nodeID);
    return true;
  }

  size_t split_varID = split_varIDs[nodeID];
  double split_value = split_values[nodeID];

// Create child nodes
  size_t left_child_nodeID = sampleIDs.size();
  child_nodeIDs[0][nodeID] = left_child_nodeID;
  createEmptyNode();
  parent_node.push_back(nodeID);
  size_t right_child_nodeID = sampleIDs.size();
  child_nodeIDs[1][nodeID] = right_child_nodeID;
  createEmptyNode();
  parent_node.push_back(nodeID);

// For each sample in node, assign to left or right child
  if ((*is_ordered_variable)[split_varID]) {
    // Ordered: left is <= splitval and right is > splitval
    for (auto& sampleID : sampleIDs[nodeID]) {
      if (data->get(sampleID, split_varID) <= split_value) {
        sampleIDs[left_child_nodeID].push_back(sampleID);
      } else {
        sampleIDs[right_child_nodeID].push_back(sampleID);
      }
    }
  } else {
    // Unordered: If bit at position is 1 -> right, 0 -> left
    for (auto& sampleID : sampleIDs[nodeID]) {
      double level = data->get(sampleID, split_varID);
      size_t factorID = floor(level) - 1;
      size_t splitID = floor(split_value);

      // Left if 0 found at position factorID
      if (!(splitID & (1 << factorID))) {
        sampleIDs[left_child_nodeID].push_back(sampleID);
      } else {
        sampleIDs[right_child_nodeID].push_back(sampleID);
      }
    }
  }
 // printf("node_sample_size %d\n",sampleIDs[nodeID].size());
// No terminal node
  return false;
}

void Tree::createEmptyNode() {
  split_varIDs.push_back(0);
  split_values.push_back(0);
  child_nodeIDs[0].push_back(0);
  child_nodeIDs[1].push_back(0);
  sampleIDs.push_back(std::vector<size_t>());

  createEmptyNodeInternal();
}

size_t Tree::dropDownSamplePermuted(size_t permuted_varID, size_t sampleID, size_t permuted_sampleID) {

// Start in root and drop down
  size_t nodeID = 0;
  while (child_nodeIDs[0][nodeID] != 0 || child_nodeIDs[1][nodeID] != 0) {

    // Permute if variable is permutation variable
    size_t split_varID = split_varIDs[nodeID];
    size_t sampleID_final = sampleID;
    if (split_varID == permuted_varID) {
      sampleID_final = permuted_sampleID;
    }

    // Move to child
    double value = data->get(sampleID_final, split_varID);
    if ((*is_ordered_variable)[split_varID]) {
      if (value <= split_values[nodeID]) {
        // Move to left child
        nodeID = child_nodeIDs[0][nodeID];
      } else {
        // Move to right child
        nodeID = child_nodeIDs[1][nodeID];
      }
    } else {
      size_t factorID = floor(value) - 1;
      size_t splitID = floor(split_values[nodeID]);

      // Left if 0 found at position factorID
      if (!(splitID & (1 << factorID))) {
        // Move to left child
        nodeID = child_nodeIDs[0][nodeID];
      } else {
        // Move to right child
        nodeID = child_nodeIDs[1][nodeID];
      }
    }

  }
  return nodeID;
}

void Tree::permuteAndPredictOobSamples(size_t permuted_varID, std::vector<size_t>& permutations) {

// Permute OOB sample
//std::vector<size_t> permutations(oob_sampleIDs);
  std::shuffle(permutations.begin(), permutations.end(), random_number_generator);

// For each sample, drop down the tree and add prediction
  for (size_t i = 0; i < num_samples_oob; ++i) {
    size_t nodeID = dropDownSamplePermuted(permuted_varID, oob_sampleIDs[i], permutations[i]);
    prediction_terminal_nodeIDs[i] = nodeID;
  }
}

void Tree::bootstrap() {

// Use fraction (default 63.21%) of the samples
  size_t num_samples_inbag = (size_t) num_samples * sample_fraction;

// Reserve space, reserve a little more to be save)
  sampleIDs[0].reserve(num_samples_inbag);
  oob_sampleIDs.reserve(num_samples * (exp(-sample_fraction) + 0.1));

  std::uniform_int_distribution<size_t> unif_dist(0, num_samples - 1);

// Start with all samples OOB
  inbag_counts.resize(num_samples, 0);

// Draw num_samples samples with replacement (num_samples_inbag out of n) as inbag and mark as not OOB
  for (size_t s = 0; s < num_samples_inbag; ++s) {
    size_t draw = unif_dist(random_number_generator);
    sampleIDs[0].push_back(draw);
    ++inbag_counts[draw];
  }

// Save OOB samples
  for (size_t s = 0; s < inbag_counts.size(); ++s) {
    if (inbag_counts[s] == 0) {
      oob_sampleIDs.push_back(s);
    }
  }
  num_samples_oob = oob_sampleIDs.size();

  if (!keep_inbag) {
    inbag_counts.clear();
  }
}

void Tree::bootstrapWeighted() {

// Use fraction (default 63.21%) of the samples
  size_t num_samples_inbag = (size_t) num_samples * sample_fraction;

// Reserve space, reserve a little more to be save)
  sampleIDs[0].reserve(num_samples_inbag);
  oob_sampleIDs.reserve(num_samples * (exp(-sample_fraction) + 0.1));

  std::discrete_distribution<> weighted_dist(case_weights->begin(), case_weights->end());

// Start with all samples OOB
  inbag_counts.resize(num_samples, 0);

// Draw num_samples samples with replacement (n out of n) as inbag and mark as not OOB
  for (size_t s = 0; s < num_samples_inbag; ++s) {
    size_t draw = weighted_dist(random_number_generator);
    sampleIDs[0].push_back(draw);
    ++inbag_counts[draw];
  }

  if (holdout) {
    for (size_t s = 0; s < (*case_weights).size(); ++s) {
      if ((*case_weights)[s] == 0) {
        oob_sampleIDs.push_back(s);
      }
    }
  } else {
    for (size_t s = 0; s < inbag_counts.size(); ++s) {
      if (inbag_counts[s] == 0) {
        oob_sampleIDs.push_back(s);
      }
    }
  }
  num_samples_oob = oob_sampleIDs.size();

  if (!keep_inbag) {
    inbag_counts.clear();
  }
}

void Tree::bootstrapWithoutReplacement() {

// Use fraction (default 63.21%) of the samples
  size_t num_samples_inbag = (size_t) num_samples * sample_fraction;
  shuffleAndSplit(sampleIDs[0], oob_sampleIDs, num_samples, num_samples_inbag, random_number_generator);
  num_samples_oob = oob_sampleIDs.size();

  if (keep_inbag) {
    // All observation are 0 or 1 times inbag
    inbag_counts.resize(num_samples, 1);
    for (size_t i = 0; i < oob_sampleIDs.size(); i++) {
      inbag_counts[oob_sampleIDs[i]] = 0;
    }
  }
}

void Tree::bootstrapWithoutReplacementWeighted() {

// Use fraction (default 63.21%) of the samples
  size_t num_samples_inbag = (size_t) num_samples * sample_fraction;
  drawWithoutReplacementWeighted(sampleIDs[0], random_number_generator, num_samples - 1, num_samples_inbag,
      *case_weights);

// All observation are 0 or 1 times inbag
  inbag_counts.resize(num_samples, 0);
  for (auto& sampleID : sampleIDs[0]) {
    inbag_counts[sampleID] = 1;
  }

// Save OOB samples. In holdout mode these are the cases with 0 weight.
  if (holdout) {
    for (size_t s = 0; s < (*case_weights).size(); ++s) {
      if ((*case_weights)[s] == 0) {
        oob_sampleIDs.push_back(s);
      }
    }
  } else {
    for (size_t s = 0; s < inbag_counts.size(); ++s) {
      if (inbag_counts[s] == 0) {
        oob_sampleIDs.push_back(s);
      }
    }
  }
  num_samples_oob = oob_sampleIDs.size();

  if (!keep_inbag) {
    inbag_counts.clear();
  }
}

