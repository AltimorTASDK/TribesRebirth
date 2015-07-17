/*
	Header file to implement Elasticity, Friction, Object ID.

	These properties are common to each different Dml type, and this approach is a
	sort of attempt to keep all the code in one place.  But you still have to insert 
	the macros below in the right places, see the macro comments below for the 
	recipe.
*/

#ifndef _COMMONPROPS_
#define	_COMMONPROPS_


#include <ts_material.h>


class CommonDmlProperties 
{
	bool	different;

public:

	CommonDmlProperties ( const TS::Material::Params * p = NULL );
	CommonDmlProperties & operator= ( const CommonDmlProperties & in_c );

	void SetPropsToParams ( const TS::Material::Params & p );
	bool GetParamsFromProps ( TS::Material::Params & p ) const;

	float OnChangeFloatEdit ( CEdit &edit );
	int OnChangeIntEdit ( CEdit &edit );

	void changeFriction (  CEdit &edit );
	void changeElasticity (  CEdit &edit );
	void changeId (  CComboBox &edit );
   void useDefaultProps(bool);

	float	m_currentFriction;
	float	m_currentElasticity;
	int	m_currentId;
   int   m_useDefaultProps;
};


/*	Here are the FIVE things you have to include to get the boxes		**
**	handled.  Additionally, you have to define the edit boxes in the	**
**	resource editor with the tags:  ID_COMMON_FRICTION#,				**
**	ID_COMMON_ELASTICITY#, and ID_COMMON_OBJECT_ID# with a unique #		**
**	for each one.														*/

// Drop this somewhere in the source file for the ClassName

#define	CommonPropertyMethodsDefined(ClassName)				\
	void ClassName::OnTexFriction()							\
	{														\
		commonProps.changeFriction ( m_frictionEdit );		\
	}														\
	void ClassName::OnTexElasticity()						\
	{														\
		commonProps.changeElasticity ( m_elasticityEdit );	\
	}														\
	void ClassName::OnTexId()								\
	{														\
		commonProps.changeId ( m_IdCombo );					\
	}                                                  \
	void ClassName::OnTexUseDefaults()								      \
	{														                     \
		commonProps.useDefaultProps(m_propsCheck.GetCheck() != 0);	\
	}                                                              \
   void ClassName::InitCommonProps()                  \
   {                                                  \
      m_IdCombo.SetCurSel(commonProps.m_currentId);   \
   }

// This goes in the protected section of the ClassName (not sure if 
// that's required).

#define	CommonPropertyMethodsDeclared()			\
	afx_msg void OnTexFriction ();				\
	afx_msg void OnTexElasticity ();			\
	afx_msg void OnTexId ();               \
	afx_msg void OnTexUseDefaults ();               \
	afx_msg void InitCommonProps ()

// Put with private control edit members.  Maybe this can be combined 
// with the previous.

#define	CommonPropertyDataMembers				\
	CommonDmlProperties		commonProps;		\
	CComboBox   m_IdCombo;						\
   CButton m_propsCheck;                  \
	CEdit	m_frictionEdit;						\
	CEdit	m_elasticityEdit

// Put in the DoDataExchange() member for ClassName.  Pass in a number which
//	uniquely identifies the ID.  

#define	CommonPropertyDataExchange(pdx, X)										      \
	DDX_Control(pdx, ID_COMMON_FRICTION##X, m_frictionEdit );					   \
	DDX_Control(pdx, ID_COMMON_ELASTICITY##X, m_elasticityEdit );				   \
   DDX_Control(pdx, ID_COMMON_OBJECT_IDC##X, m_IdCombo );                     \
   DDX_Control(pdx, ID_COMMON_USEDEFAULTS##X, m_propsCheck );             \
	DDX_Check(pdx, ID_COMMON_USEDEFAULTS##X, commonProps.m_useDefaultProps );	\
	DDX_Text(pdx, ID_COMMON_FRICTION##X, commonProps.m_currentFriction );		\
	DDV_MinMaxFloat(pdx, commonProps.m_currentFriction, 0.0, 1000000000.0 );	\
	DDX_Text(pdx, ID_COMMON_ELASTICITY##X, commonProps.m_currentElasticity );	\
	DDV_MinMaxFloat ( pdx, commonProps.m_currentElasticity, 0.0, 1000000000.0 );

// Put in the message map for ClassName.

#define	CommonPropertyMessageMap(X)								\
	ON_EN_CHANGE(ID_COMMON_FRICTION##X, OnTexFriction )			\
	ON_EN_CHANGE(ID_COMMON_ELASTICITY##X, OnTexElasticity )		\
	ON_BN_CLICKED(ID_COMMON_USEDEFAULTS##X, OnTexUseDefaults )		\
	ON_CBN_SELCHANGE(ID_COMMON_OBJECT_IDC##X, OnTexId )

#endif
