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

    bool is_applicable = false;

    if ( ntk.is_on_critical_path( n ) ){
      is_applicable = true;
      ntk.foreach_fanin( n,
                         [&]( signal const& s1 ) {             // look at the 2 fan in signals of n
                           node n_parent = ntk.get_node( s1 ); // take the node n_parent pointed by signal s1
                           if ( ntk.is_on_critical_path( n_parent ) ) // if n_parent is on the critical path
                           {
                             s1_critical = s1;
                             if ( !ntk.is_complemented( s1_critical ) ) // if s1 is not complemented ( AND gate, not a NAND, associativity applicable)
                             {
                               // check with input is on the critical path
                               ntk.foreach_fanin( n_parent,
                                                  [&]( signal const& s2 ) {
                                                    node n_input = ntk.get_node(s2); // node pointed by s2
                                                    if(ntk.is_on_critical_path(n_input)){ // if n_input is on the critical path
                                                      s2_critical = s2;
                                                      ntk.substitute_node(n_input, s2_critical); // node n_input is substituted by signal s2_critical
                                                    }
                                                    else
                                                      s2_non_critical = s2
                                                  } )
                             }
                             else
                               return (is_applicable = false); // Associativity is not applicable, the node on the critical path is complemented (NAND)
                           }
                           else
                             s1_non_critical = s1;
                         } )
    }

    if (is_applicable){
      new_and = ntk.create_and(s1_non_critical, s2_non_critical); // creation of the new node
      s1_non_critical = new_and;
      // s1_critical = s2_critical;
      return true;
    }
    else
      return false;
  }

  /* Try the distributivity rule on node n. Return true if the network is updated. */
  bool try_distributivity( node n )
  {
    /* TODO */
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