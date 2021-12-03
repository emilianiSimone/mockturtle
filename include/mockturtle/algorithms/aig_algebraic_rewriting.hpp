/*!
  \file aig_algebraic_rewriting.hpp
  \brief AIG algebraric rewriting

  EPFL CS-472 2021 Final Project Option 1
*/

#pragma once

#include "../networks/aig.hpp"
#include "../views/depth_view.hpp"
#include "../views/topo_view.hpp"

namespace mockturtle
{

namespace detail
{

template<class Ntk>
class aig_algebraic_rewriting_impl
{
  using node = typename Ntk::node;
  using signal = typename Ntk::signal;

public:
  aig_algebraic_rewriting_impl( Ntk& ntk )
      : ntk( ntk )
  {
    static_assert( has_level_v<Ntk>, "Ntk does not implement depth interface." );
  }

  void run()
  {
    bool cont{ true }; /* continue trying */
    while ( cont )
    {
      cont = false; /* break the loop if no updates can be made */
      ntk.foreach_gate( [&]( node n )
                        {
                          if ( try_algebraic_rules( n ) )
                          {
                            ntk.update_levels();
                            cont = true;
                          }
                        } );
    }
  }

private:
  /* Try various algebraic rules on node n. Return true if the network is updated. */
  bool try_algebraic_rules( node n )
  {
    if ( try_associativity( n ) )
      return true;
    if ( try_distributivity( n ) )
      return true;
    /* TODO: add more rules here... */

    return false;
  }

  /* Try the associativity rule on node n. Return true if the network is updated. */
  bool try_associativity( node n )
  {
    /* TODO */
    
    signal s2_critical, s2_non_critical, s1_critical, s1_non_critical, new_and;

    bool is_appl = false;                  // is applicable ?
    bool is_adv1 = false, is_adv2 = false; // is advantageous ? (primary and secondary fan-in)
    uint32_t level_critical = 0, level_non_critical = 0;
    
    if ( !ntk.is_on_critical_path( n ) )
    {
      return false;
    }

    // check for applicability and benefit
    ntk.foreach_fanin( n, [&]( signal const& s1 ) { // access to the 2 primary fan-in signal
      node child1 = ntk.get_node( s1 );
      if ( ntk.is_on_critical_path( child1 ) )
      { // check critical path of fan-in
        s1_critical = s1;
        level_critical = ntk.level( child1 ); // save level of critical node
        if ( !ntk.is_complemented( s1 ) ) // if not complemented, associativity is applicable
        { 
          is_appl = true;
          ntk.foreach_fanin( child1, [&]( signal const& s2 ) { // access to the 2 secondary fan-in signal
            if ( ntk.is_on_critical_path( ntk.get_node( s2 ) ) )
            {
              s2_critical = s2;
              // if only one secondary fan-in node is on critical path associativity can be advantageous
              // if both secondary fan-in nodes are on critical path associativity is not advantageous
              if ( !is_adv2 )
              {
                is_adv2 = true;
              }
              else
              {
                is_adv2 = false;
              }
            }
            else
            {
              s2_non_critical = s2;
            }
            return;
          } );
        }

        // if only one primary fan-in node is on critical path associativity can be advantageous
        // if both primary fan-in nodes are on critical path associativity is not advantageous
        if ( !is_adv1 )
        {
          is_adv1 = true;
        }
        else
        {
          is_adv1 = false;
        }
      }
      else
      {
        s1_non_critical = s1;
        level_non_critical = ntk.level( child1 );
      }
      return;
    } );

    if ( is_adv1 ) // check on the levels of primary fan-in
    {
      if ( level_non_critical + 1 >= level_critical)
      {
        is_adv1 = false;
      }
    }
    
    // implementation of associativity
    if ( is_appl && is_adv1 && is_adv2 )
    {
      signal new_and1 = ntk.create_and( s1_non_critical, s2_non_critical ); // new and between non critical signals
      signal new_and2 = ntk.create_and( new_and1, s2_critical );            // new and that will replace node n
      ntk.substitute_node( n, new_and2 );                                   // substitution
      return true;
    }
    
    return false;
  }

  /* Try the distributivity rule on node n. Return true if the network is updated. */
  bool try_distributivity( node n )
  {
    /* TODO */

      /*
    bool is_appl = true; // is applicable?

    if ( !ntk.is_on_critical_path( n ) )
    {
      return false;
    }

    // check for applicability and benefit
    ntk.foreach_fanin( n, [&]( signal const& s1 )
                       {
                         node child1 = ntk.get_node( s1 );
                         if ( ntk.is_on_critical_path( child1 ) )
                         {
                           if ( ntk.is_complemented( s1 ) )
                           {

                             // if both primary inputs are complemented, then distributivity is applicable
                             // note that here the logic is reversed wrt associativity (true-false-true)
                             if ( is_appl )
                             {
                               is_appl = false;
                             }
                             else
                             {
                               is_appl = true;
                             }
                           }
                         }
                         return;
                       } );
                       */

      return false;
  }

private:
  Ntk& ntk;
};

} // namespace detail

/* Entry point for users to call */
template<class Ntk>
void aig_algebraic_rewriting( Ntk& ntk )
{
  static_assert( std::is_same_v<typename Ntk::base_type, aig_network>, "Ntk is not an AIG" );

  depth_view dntk{ ntk };
  detail::aig_algebraic_rewriting_impl p( dntk );
  p.run();
}

} /* namespace mockturtle */