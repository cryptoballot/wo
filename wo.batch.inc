<?php
/**
 * Batch operations
 * 
 * These functions are run occationally (on cron) to update various metrices such 
 * as consensus ratings, vote totals etc.
 */

function wo_batch_update_issue($issue) {
  $results = wo_calculate_vote($issue->nid);
  $issue->field_wo_issue_proposals['und'] = array();
  foreach ($results['list'] as $proposal_nid) {
    $issue->field_wo_issue_proposals['und'][]['target_id'] = $proposal_nid;
  }
  $issue->field_wo_issue_consensus['und'][0]['value'] = $results['consensus'];
  node_save($issue);
}