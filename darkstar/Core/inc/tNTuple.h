//
// tNTuple.h
//

#ifndef _TNTUPLE_H_
#define _TNTUPLE_H_

#ifdef __BORLANDC__
#pragma option -w-inl
#endif



template <size_t N, class T>
class NTuple 
{
public:
   T  vec [ N ];
   
   NTuple & operator += ( NTuple & in );
   NTuple & operator -= ( NTuple & in );
   // NTuple & operator += ( T amt );
   // NTuple & operator -= ( T amt );
   NTuple & operator <<= ( int amt );
   NTuple & operator >>= ( int amt );
   NTuple & operator *=  ( int amt );
   NTuple & operator /=  ( int amt );
   T & operator [] ( int i ) { return vec[i]; }
   NTuple & operator = ( T value );
};


template<size_t N, class T> inline 
NTuple<N,T>&  NTuple<N,T>::operator += ( NTuple & in ){
   for( int i = N-1; i >= 0; i-- )
      vec[ i ] += in.vec[ i ]; 
   return( *this );
}
template<size_t N, class T> inline 
NTuple<N,T>&  NTuple<N,T>::operator -= ( NTuple & in ){
   for( int i = N-1; i >= 0; i-- )
      vec[ i ] -= in.vec[ i ]; 
   return( *this );
}


template<size_t N, class T> inline 
NTuple<N,T>&  NTuple<N,T>::operator <<= ( int amt ){
   for( int i = N-1; i >= 0; i-- )
      vec[ i ] <<= amt;
   return ( *this );
}
template<size_t N, class T> inline 
NTuple<N,T>&  NTuple<N,T>::operator >>= ( int amt ){
   for( int i = N-1; i >= 0; i-- )
      vec[ i ] >>= amt;
   return ( *this );
}


template<size_t N, class T> inline 
NTuple<N,T>&  NTuple<N,T>::operator *= ( int amt ){
   for( int i = N-1; i >= 0; i-- )
      vec[ i ] *= amt;
   return ( *this );
}
template<size_t N, class T> inline 
NTuple<N,T>&  NTuple<N,T>::operator /= ( int amt ){
   for( int i = N-1; i >= 0; i-- )
      vec[ i ] /= amt;
   return ( *this );
}

// assign all to the same value.  
template<size_t N, class T> inline 
NTuple<N,T>&  NTuple<N,T>::operator = ( T value ){
   for( int i = N-1; i >= 0; i-- )
      vec[ i ] = value;
   return ( *this );
}


#ifdef __BORLANDC__
#pragma option -winl.
#endif


#endif
