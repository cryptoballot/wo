<?php
/**
 * Calculate Vote
 * 
 * Calculate the votes for a given issue using the ranked-pairs condorcet method
 * See http://en.wikipedia.org/wiki/Condorcet_method and http://en.wikipedia.org/wiki/Ranked_pairs
 * 
 * Note that we are implementing the ranked-pairs algorithm for condorcet calculations
 * because it is the simplest to implement. However, it does not scale well and suffers
 * from several other drawbacks. In the future we should look into other condorcet systems.
 * @@TODO: Move this to a go, D, or erlang service. Hadoop?
 */
function wo_calculate_vote($issue_nid) {
  $votes = wo_issue_get_votes($issue_nid);
  
  // Create a proposal array that holds all the proposals
  $proposals = array();
  //@@TODO: Entity-field query
  $props_db = db_query('SELECT entity_id as nid FROM field_data_field_wo_proposal_issue WHERE field_wo_proposal_issue_target_id = :bid', array(':bid' => $issue_nid));
  foreach ($props_db as $prop_db) {
    $proposals[] = $prop_db->nid;
  }
  
  // Special-case: If there is only one proposal, return it as the winner
  if (count($proposals) == 1) {
    $proposal = $proposals[0];
    return array(
      'winner' => $proposal,
      'list' => array($proposal),
      'consensus' => wo_calculate_vote_consensus($proposal, $votes),
    );
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
            $pair['prefer_a']++;
            break;
          }
          if ($item == $prop_b) {
            $pair['prefer_b']++;
            break;
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
    foreach ($pair['props'] as $prop_key => $prop) {
      // If this proposition hasn't been seen in the graph before, set it up 
      if (!isset($graph[$prop])) {
        $graph[$prop] = array(
          'prop' => $prop,
          'parents' => array(),
          'children' => array(),
          'siblings' => array(),
        );
      }
      // Set the parent / child / sibling relationship in the graph based on winner / loser in the pair
      if ($pair['winner'] === FALSE) {
        $graph[$prop]['siblings'][] = $pair['props'][($prop_key == 'a' ? 'b' : 'a')];
      }
      if ($pair['winner'] == $prop) {
        $graph[$prop]['children'][] = $pair['loser'];
      }
      if ($pair['loser'] == $prop) {
        $graph[$prop]['parents'][] = $pair['winner'];
      }
    }
  }
  
  // Compile the results (winner, ranked list, consensus ratio, and graph)
  $results = array(
    'winner' => FALSE,
    'list' => array(),
    'consensus' => 0,
    'graph' => $graph,
  );
  
  // Calculate the winner. If there is no winner it implies that there is no transitive proposal
  if ($winner = wo_calculate_vote_find_winner($graph)) {
    $results['winner'] = $winner;
  }
  
  // If there is no winner, find the pseudo-winner to place at the top of the list
  // Walk the tree upwards until we hit a loop
  if (!$winner) {
    $winner = wo_calculate_vote_find_pseudo_winner($graph);
  }
  
  // To create the ranked proposal list, walk the tree from the winner downwards
  $results['list'] = wo_calculate_vote_create_list($graph, $winner);
  
  // Calculate consensus ratio
  $results['consensus'] = wo_calculate_vote_consensus($winner, $votes);
  
  // Uncomment this to debug
  #dpm($votes);
  #dpm($tally);
  #dpm($winner);
  #dpm($graph); 
  
  return $results;
}


/**
 * Calculate Vote - Sort Ranked Pairs callback
 * 
 * usort() callback. Properly ranks pairs
 */
function wo_calculate_vote_rank_pairs($pair_a, $pair_b) {
  return ($pair_a['amount'] - $pair_b['amount']);
}

/**
 * Calculate Vote - Find the winner
 * 
 * Given a graph, find the winner. A winner is defined as the node with no parents
 */
function wo_calculate_vote_find_winner($graph) {
  foreach ($graph as $node) {
    if (empty($node['parents'])) {
      if (empty($node['siblings'])) {
        return $node['prop'];
      }
      else {
        // The winner has siblings, therefore non-transitive
        return FALSE;
      }
    }
  }
}

/**
 * Calculate Vote - Find pseudo-winner
 * 
 * The pseudo-winner can be defined as any member of the smith-set
 * See http://en.wikipedia.org/wiki/Smith_set
 * 
 * We fine the pseudo-winner by walking the tree upwards via parents,
 * eliminating nodes as we go, until we arrive at a node with no parents
 * that have not already been eliminated. This node is guaranteed to be 
 * in the smith set.
 */
function wo_calculate_vote_find_pseudo_winner($graph, $node = FALSE) {
  if (!$node) {
    $node = array_pop($graph);
  }

  if (empty($node['parents'])) {
    //@@TODO: There might be more work here to check siblings
    // We should be returning the sibling with the highest number of children
    // If they have the same number of children, then the highest number of vote-diff
    return $node['prop'];
  }
  else {
    // Walk up the tree by just transversing via the parents
    foreach ($node['parents'] as $pid) {
      if (isset($graph[$pid])) {
        $parent = $graph[$pid];
        unset($graph[$pid]);
        return wo_calculate_vote_find_pseudo_winner($graph, $parent);
      }
    }
  }
}

/**
 * Calculate Vote - Create ordered list of proposals
 * 
 * Walk the graph, creating an ordered list of proposals.
 * If there are self-referencing loops in the graph, this list can
 * be considered a full-fidelity representation of the graph. 
 * If there are loops in the graph, a winner for the local-loop
 * is programatically chosen.
 * 
 * Eventually this should be updated so that loops are explicitly detected
 * and their members sorted randomly. 
 */
function wo_calculate_vote_create_list($graph, $winner) {
  $list = array();
  wo_calculate_vote_create_list_walk($graph, $list, $winner);
  return $list;
}

/**
 * Calculate Vote - Create ordered list of proposals - callback
 * 
 * Callback function to walk the graph and create the list
 */
function wo_calculate_vote_create_list_walk(&$graph, &$list, $prop) {
  // If this node is not in the graph, skip it
  if (!isset($graph[$prop])) return;

  // Add the node to the list
  $list[] = $prop; 

  // Grab the node from the graph
  $node = $graph[$prop];
  
  // Unset the node, marking it as processed
  unset($graph[$prop]);

  // Walk the graph. First walk parents, then siblings, then children

  // Generally there will be no valid parents, but there sometimes will be if there
  // is a circular reference in the graph. This ensures we clear out all circular references
  foreach ($node['parents'] as $parent) {
    wo_calculate_vote_create_list_walk($graph, $list, $parent);
  }

  // Siblings
  foreach ($node['siblings'] as $sib) {
    wo_calculate_vote_create_list_walk($graph, $list, $sib);
  }

  // For the children, only process the children that have no unprocessed parents, this
  // ensures that a child is only processed after all parents are processed
  foreach ($node['children'] as $child) {
    if (isset($graph[$child])) {
      $child_has_parents = FALSE;
      $child_node = $graph[$child];
      foreach ($child_node['parents'] as $child_node_parent) {
        if (isset($graph[$child_node_parent])) {
          $child_has_parents = TRUE;
          break;
        }
      }
      if (!$child_has_parents) {
        wo_calculate_vote_create_list_walk($graph, $list, $child);
      }
    }
  }
}

/**
 * Calculate Vote - Calculate consensus the index
 * 
 * The consensus ratio is composed as the product of two numbers:
 * 1. The percent of the total number of users that have voted on this issue (voter turnout)
 * 2. The (inverse) average position of the winner in each vote
 * 
 * If every eligible voter has voted and ranked the winner as their first preference, then
 * the consensus ratio would be 1
 * 
 * If there are no votes, then the consensus ratio is 0. Equally, a circular 
 */
function wo_calculate_vote_consensus($winner, $votes) {
  $voter_turnout = count($votes) / wo_total_voters();
  
  $sum = 0;
  foreach ($votes as $vote) {
    $position = array_search($winner, $vote);
    // If the voter didn't rank the winner, consider it having been placed
    // just after their worst ranked proposal
    if ($position === FALSE) {
      $position = count($vote);
    }
    // Add to the sum for the average
    $sum = $sum + $position + 1;
  }
  
  $average = $sum / count($votes);
  $inverse_average = 1 / $average;
  
  $raw_consensus = $voter_turnout * $inverse_average;
  
  #$scaled_consensus = .. // Balance voter-turnout vs. inverse average
  return $inverse_average;
}

/**
 * Get votes for an issue
 * 
 * Grab the vote nodes and create a compacted representation of the votes for use in other functions
 */
function wo_issue_get_votes($issue_nid) {
  // Create a vote array that holds all the votes
  $votes = array();
  $votes_db = db_query('SELECT entity_id as nid FROM field_data_field_wo_vote_issue WHERE field_wo_vote_issue_target_id = :bid', array(':bid' => $issue_nid));
  foreach ($votes_db as $vote_db) {
    $vote_node = node_load($vote_db->nid);
    $votes[$vote_node->nid] = array();
    foreach ($vote_node->field_wo_vote_rank['und'] as $item) {
      $votes[$vote_node->nid][] = $item['target_id'];
    }
  }
  return $votes;
}

/**
 * Total Voters
 * 
 * Calculate the total number of active votes
 */
function wo_total_voters() {
  //@@TODO: Return the proper amount based on role
  $result = db_query('SELECT COUNT(uid) as count from users WHERE status = 1')->fetchField();
  return $result['count'];
}