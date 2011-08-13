<?php
/**
 * Calculate Vote
 * 
 * Calculate the votes for a given issue using the ranked-pairs condorcet method
 * See http://en.wikipedia.org/wiki/Ranked_pairs
 * 
 * Note that we are implementing the ranked-pairs algorithm for condorcet calculations
 * because it is the simplest to implement. However, it does not scale well and suffers
 * from several other drawbacks. In the future we should look into other condorcet systems.
 */
function wo_calculate_vote($issue_nid) {
  // Create a vote array that holds all the votes
  $votes = array();
  $votes_db = db_query('SELECT entity_id as nid FROM field_data_field_wo_vote_issue WHERE field_wo_vote_issue_nid = :bid', array(':bid' => $issue_nid));
  foreach ($votes_db as $vote_db) {
    $vote_node = node_load($vote_db->nid);
    $votes[$vote_node->nid] = array();
    foreach ($vote_node->field_wo_vote_rank['und'] as $item) {
      $votes[$vote_node->nid][] = $item['nid'];
    }
  }
  
  // Create a proposal array that holds all the proposals
  $proposals = array();
  $props_db = db_query('SELECT entity_id as nid FROM field_data_field_wo_proposal_issue WHERE field_wo_proposal_issue_nid = :bid', array(':bid' => $issue_nid));
  foreach ($props_db as $prop_db) {
    $proposals[] = $prop_db->nid;
  }
  
  // Ranked-pairs Step 1: Tally
  // For-each proposal pair, determine who the winner is, and by what amount
  $tally = array();
  foreach ($proposals as $prop_a) {
    foreach ($proposals as $prop_b) {
      
      // If they are the same, or this combination has already been done, skip it
      if ($prop_a == $prop_b) continue;
      if (isset($tally[$prop_b.'_'.$prop_a])) continue;
      
      // Create an array summarizing the pair
      $pair = array(
        'props' => array(
          'a' => $prop_a,
          'b' => $prop_b,
        ),
        'prefer_a' => 0,
        'prefer_b' => 0,
        'winner' => FALSE,
        'loser' => FALSE,
        'amount' => 0,
      );
      
      // Count the votes
      foreach ($votes as $vote) {
        foreach ($vote as $item) {
          if ($item == $prop_a) {
            $pair['prefer_a'] = $pair['prefer_a'] + 1;
            continue;
          }
          if ($item == $prop_b) {
            $pair['prefer_b'] = $pair['prefer_b'] + 1;
            continue;
          }
        }
      }
      
      // Determine the winner and looser (if neither is preferred, both stay FALSE)
      if ($pair['prefer_a'] > $pair['prefer_b']) {
        $pair['winner'] = $prop_a;
        $pair['loser'] = $prop_b;
      }
      if ($pair['prefer_b'] > $pair['prefer_a']) {
        $pair['winner'] = $prop_b;
        $pair['loser'] = $prop_a;
      }
      
      // If there's a winner, calculate the amount they won by
      if ($pair['winner']) {
        $pair['amount'] = abs($pair['prefer_a'] - $pair['prefer_b']);
      }
      
      // Add the pair to the tally
      $tally[$prop_a.'_'.$prop_b] = $pair;
    }
  }
  
  // Ranked-pairs Step 2: Sort
  uasort($tally, 'wo_calculate_vote_rank_pairs');
  
  // Ranked-pairs Step 2: Graph
  $graph = array();
  foreach ($tally as $pair) {
    foreach ($pair['props'] as $prop) {
      // If this proposition hasn't been seen in the graph before, set it up 
      if (!isset($graph[$prop])) {
        $graph[$prop] = array(
          'parents' => array(),
          'children' => array(),
        );
      }
      // Set the parent / child relationship in the graph based on winner / loser in the pair
      if ($pair['winner'] == $prop) {
        $graph[$prop]['children'][] = $pair['loser'];
      }
      if ($pair['loser'] == $prop) {
        $graph[$prop]['parents'][] = $pair['winner'];
      }
    }
  }
  
  // To determine the winner, find the node with no parents.
  foreach ($graph as $prop => $node) {
    if (empty($node['parents'])) return $prop;
  }
  
  // If there is no node with no parents, then we do not have a transitive choice. return FALSE
  return FALSE;
}



function wo_calculate_vote_rank_pairs($pair_a, $pair_b) {
  return ($pair_b['amount'] - $pair_a['amount']);
}