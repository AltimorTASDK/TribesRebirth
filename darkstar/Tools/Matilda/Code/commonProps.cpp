/*
	See commonProps.h for documentation.  
*/


#include "stdafx.h"
#include "commonProps.h"
#include "matilda2.h"


CommonDmlProperties::CommonDmlProperties ( const TS::Material::Params * p )
{
	if ( p == NULL )
	{
		different = true;
		m_currentFriction = 1.0;
		m_currentElasticity = 1.0;
		m_currentId = 0;
	}
	else
	{
		different = false;
		SetPropsToParams ( *p );
		// m_currentFriction = p->fFriction;
		// m_currentElasticity = p->fElasticity;
		// m_currentId = p->fType;
	}
}




void CommonDmlProperties::SetPropsToParams ( const TS::Material::Params & p )
{
	m_currentFriction = p.fFriction;
	m_currentElasticity = p.fElasticity;
	m_currentId = p.fType;
   m_useDefaultProps = (p.fUseDefaultProps != 0) ? 1 : 0;
}


/* Define our copy.  The other members should get init'd OK with their
			own constructors.  
*/

CommonDmlProperties & CommonDmlProperties::operator= ( 
							const CommonDmlProperties & in_c )
{
	m_currentFriction = in_c.m_currentFriction;
	m_currentElasticity = in_c.m_currentElasticity;
	m_currentId = in_c.m_currentId;
	m_useDefaultProps = in_c.m_useDefaultProps;
	different = in_c.different;

	return ( *this );
}


bool CommonDmlProperties::GetParamsFromProps ( TS::Material::Params & p ) const
{
	bool		b1, b2, b3, b4;

	if ( b1 = p.fType != m_currentId )
		p.fType = m_currentId;
	if ( b2 = p.fFriction != m_currentFriction )
		p.fFriction = m_currentFriction;
	if ( b3 = p.fElasticity != m_currentElasticity )
		p.fElasticity = m_currentElasticity;
	if ( b4 = ((p.fUseDefaultProps != 0) != (m_useDefaultProps != 0)) )
		p.fUseDefaultProps = (m_useDefaultProps != 0) ? 1 : 0;

	return ( b1 || b2 || b3 || b4);
}


int CommonDmlProperties::OnChangeIntEdit(CEdit &edit)
{
	CString		buffer;
	int			returnVal;

	edit.GetWindowText(buffer);
	if (buffer.GetLength() == 0)
		return 0;
	else 
		sscanf(buffer, "%u", &returnVal);

	return (BYTE)returnVal;
}


float CommonDmlProperties::OnChangeFloatEdit(CEdit &edit)
{
	CString		buffer;
	float		returnVal;

	edit.GetWindowText(buffer);
	if (buffer.GetLength() == 0)
		return 0;
	else 
		sscanf(buffer, "%f", &returnVal);

	return returnVal;
}



void CommonDmlProperties::changeFriction (  CEdit &edit )
{
	m_currentFriction = OnChangeFloatEdit ( edit );
	different = true;
}
void CommonDmlProperties::changeElasticity (  CEdit &edit )
{
	m_currentElasticity = OnChangeFloatEdit ( edit );
	different = true;
}
void CommonDmlProperties::changeId (  CComboBox &cBox )
{
	m_currentId = cBox.GetCurSel();
	different = true;
}

void
CommonDmlProperties::useDefaultProps(bool in_use)
{
   m_useDefaultProps = in_use ? 1 : 0;
   different = true;
}


