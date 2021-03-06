/*  
 * Python Notice:
 * Created on Apr 15, 2011
 * @author: kural
 *
 * Functions here contain the graph alignment algorithm,
 * implemented in the block-matrix approach.
 *
 * nodealign.cpp
 * glia
 *
 * Created by Deniz Kural on 6/28/11.
 * Copyright 2011 Deniz Kural. All rights reserved.
 *
 */

#include "nodealign.h"

using namespace std;


/* 
 * Declare Data Structures
 * TODO: Some to graduate into classes?
 */


// compare parent nodes by score.
struct cmp_parent_nodes {
    int y_i;
    bool operator () (const sn* node_x, const sn* node_y) {
        return node_x->matrix[y_i][node_x->seq_len].score > node_y->matrix[y_i][node_y->seq_len].score;
    }
    cmp_parent_nodes(int i) : y_i(i) { }
};


/*
 * INPUTS:  read sequence, read length, node, alignment parameters
 * OUTPUTS: alignment
 *
 * TODO:    Accept node pointer instead?
 * TODO:    Pass outputs as inputs-by-reference
 * TODO:    Check consistency of char vs int
 *
 */
int StringNodeAlign(string& read, int read_length, sn &node, int match, int mism, int gap) {
    /* Main Alignment Algorithm for String and Node */
	
    // Initialize variables
    int dia_score;
    int dia;
	
    sn* top_parent;					// pointer or not)?

	
    // Initialize Top Score before the alignment starts
    ts top_score; // now done in constructor

    ms o;
    o.parent = &node;
    vector<vector<ms> > matrix (read_length+1, vector<ms>(node.seq_len+1, o));

    /* Inherit The Top values for the leftmost utility column */
	
    // Seed the Service Left Column to 0 or parents
    if (node.p5.empty()) {
        for (int i = 0; i < read_length +1; i++) {
            matrix[i][0].parent = &node; }                // redundant, (replace with what?)
    } else {
        for (int i = 1; i < read_length + 1; i++) {
            // i = 1 because coordinate [0,0] gets seeded with the top row above
		
            // THIS LINE BELOW IS NOT PORTED!!!
            //top_parent = sorted([ [p.mscore[i][-1], p.mscore[i][-1][0] ] for p in node.p5] key=itemgetter(1), reverse=True)[0][0]
			
            //cmp_parent_nodes.y_i = i;
            cmp_parent_nodes my_compare(i);
            sort(node.p5.begin(), node.p5.end(), my_compare);

            top_parent = node.p5[0];
			
            // DEBUG PRINT
            // XXX ... the top parent is the null; but...
            //displayAlignment(top_parent);
            /*
              cout << endl;
              cout << "node p5's for " << node.name << endl;
              int q = 1;
              for (vector<sn*>::iterator n = node.p5.begin();
              n != node.p5.end(); ++n, ++q) {
              cout << q << " y="
              << (*n)->matrix[i][(*n)->seq_len].score
              << " " << *n << endl;
              }
            */
            //cout << "top parent: " << top_parent->name;
            //cout << "score at y:" << i << "x:" << top_parent->seq_len << " -- "; 
            //cout << top_parent->matrix[i][top_parent->seq_len].score<<endl;

            //cerr << "top parent matrix x " << top_parent->matrix.size() << endl;
            //cerr << "top parent matrix y " << top_parent->matrix.front().size() << endl;
			
            matrix[i][0].score = top_parent->matrix[i][top_parent->seq_len].score;  // convert to pointers?
            matrix[i][0].parent = top_parent;
            matrix[i][0].arrow = top_parent->matrix[i][top_parent->seq_len].arrow;  // convert to pointers?			
			
            //Update Top Score
            if (matrix[i][0].score > top_score.score) {
                top_score.score = matrix[i][0].score;
                top_score.y = i;
                top_score.x = 0;
            }
        }
    }

    //cerr << "set up for the alignment" << endl;
	
    /* Do the Alignment */
    for (int y = 1; y < read_length + 1; y++) {
        for (int x = 1; x < node.seq_len + 1; x++) {
				
            /* Figure out if the base is a match or a mismatch
               Note that the indexes x-1 and y-1 refer to the sequences and not to the matrix
               The first letters indexed at 0 correspond to  (1,1)  on the matrix */
			
            // debug print
            //if (node.name == "chr10_20m_1kb:173-288.alt.1") {
            //    cout << node.sequence << endl;
            //    cout << node.sequence[x-1] << ":" << x-1 << " " << read[y-1] << ":" << y-1 << endl;
            //}
			
            if (node.sequence[x-1] == read[y-1]) {
                dia_score = match;
                dia = 'm';
            } else {
                dia_score = mism;
                dia = 'x';
            }

            /* Calculate the potential scores */
				
            // debug prints
            // cout node.mscore[y][x];
            // cout node.mscore[y-1][x-1];
            // cout node.mscore[y][x-1];

            int dia_total = matrix[y-1][x-1].score + dia_score;
            int up_total = matrix[y-1][x].score + gap;
            int side_total = matrix[y][x-1].score + gap;
			
			
            // Compare scores & update matrices (score + arrow)
            // todo: Add gap_extend
            // todo: Separate out the update function to a small function..
			
            if (dia_total >= up_total) {
                if (dia_total >= side_total) {
                    if (dia_total < 0)
                        dia_total = 0;
                    matrix[y][x].arrow = dia;
                    matrix[y][x].score = dia_total;
                } else {
                    if (side_total < 0)
                        side_total = 0;
                    matrix[y][x].arrow = 's';
                    matrix[y][x].score = side_total;
                }
            } else if (up_total >= side_total) {
                if (up_total < 0)
                    up_total = 0;
                matrix[y][x].arrow = 'u';
                matrix[y][x].score = up_total;		
            } else {
                if (side_total < 0)
                    side_total = 0;
                matrix[y][x].arrow = 's';
                matrix[y][x].score = side_total;
            }

            /* Assign the matrix
               the format:  score, arrow, node.id
               py: node.mscore[y][x] = [score[1], score[0], node.id, node]  */

            /* Update Top Score */
            if (matrix[y][x].score > top_score.score) {
                top_score.score = matrix[y][x].score;
                top_score.y = y;
                top_score.x = x;
            }
		
        }	// close the row
    }	// close the column
			
    node.top_score = top_score;
    node.matrix = matrix;
    //cerr << "done with StringNodeAlign" << endl;
    return 0;
}
