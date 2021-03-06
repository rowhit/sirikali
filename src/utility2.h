/*
 *
 *  Copyright (c) 2017
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILITY_TWO_H
#define UTILITY_TWO_H

#include <QStringList>
#include <QDir>
#include <QtGlobal>

#include <functional>
#include <memory>
#include <type_traits>

class QByteArray ;
class QTranslator ;

namespace utility2
{
	template< typename Type >
	Type reflect_argument( Type e )
	{
		return e ;
	}

	template< typename Type,typename ... Arguments >
	Type * create_type( Arguments&& ... args )
	{
		if( sizeof ... ( args ) == 0 ){

			return new Type() ;
		}else{
			return new Type( std::forward< Arguments >( args ) ... ) ;
		}
	}

	/*
	 * This method takes a function that returns a resource,a function that deletes
	 * the resource and arguments that are to be passed to the function that returns a
	 * resource.
	 *
	 * example usecase of a function:
	 *
	 * auto woof = utility2::unique_rsc( ::fopen,::fclose,"/woof/foo/bar","r" ) ;
	 */
	template< typename Function,typename Deleter,typename ... Arguments >
	auto unique_rsc( Function&& function,Deleter&& deleter,Arguments&& ... args )
	{
		using A = std::remove_pointer_t< std::result_of_t< Function( Arguments&& ... ) > > ;
		using B = std::decay_t< Deleter > ;

		return std::unique_ptr< A,B >( function( std::forward< Arguments >( args ) ... ),
					       std::forward< Deleter >( deleter ) ) ;
	}

	template< typename Type,typename Deleter,typename ... Arguments >
	auto unique_ptr( Deleter&& deleter,Arguments&& ... args )
	{
		return unique_rsc( utility2::create_type< Type >,
				   std::forward< Deleter >( deleter ),
				   std::forward< Arguments >( args ) ... ) ;
	}

	template< typename Type,typename Deleter >
	auto unique_ptr( Type type,Deleter&& deleter )
	{
		return unique_rsc( utility2::reflect_argument< Type >,
				   std::forward< Deleter >( deleter ),type ) ;
	}

	template< typename Type,typename ... Arguments >
	auto unique_qptr( Arguments&& ... args )
	{
		return utility2::unique_ptr< Type >( []( Type * e ){ e->deleteLater() ; },
						     std::forward< Arguments >( args ) ... ) ;
	}

	template< typename Type >
	auto unique_qptr( Type e )
	{
		return unique_rsc( utility2::reflect_argument< Type >,
				   []( Type e ){ e->deleteLater() ; },e ) ;
	}

	namespace detail
	{
		template< typename E,typename F,typename G >
		void stringListToStrings( const F& s,G n,G k,E& e )
		{
			if( n < k ){

				e = s.at( n ) ;
			}
		}

		template< typename E,typename F,typename G,typename ... T >
		void stringListToStrings( const E& s,G n,G k,F& e,T& ... t )
		{
			if( n < k ){

				e = s.at( n ) ;
				utility2::detail::stringListToStrings( s,n + 1,k,t ... ) ;
			}
		}
	}

	template< typename E,typename ... F >
	void stringListToStrings( const E& s,F& ... t )
	{
		using type_t = decltype( s.size() ) ;

		utility2::detail::stringListToStrings( s,type_t( 0 ),s.size(),t ... ) ;
	}

	static inline QStringList executableSearchPaths()
	{
		const auto a = QDir::homePath().toLatin1() ;
		const auto b = a + "/bin/" ;
		const auto c = a + "/.bin/" ;

#ifdef Q_OS_WIN
		return { b.constData(),c.constData() } ;
#else
		return { "/usr/local/bin/",
			"/usr/local/sbin/",
			"/usr/bin/",
			"/usr/sbin/",
			"/bin/",
			"/sbin/",
			"/opt/local/bin/",
			"/opt/local/sbin/",
			"/opt/bin/",
			"/opt/sbin/",
			 b.constData(),
			 c.constData() } ;
#endif
	}

	static inline QString executableFullPath( const QString& f,
						  std::function< QString( const QString& ) > function = nullptr )
	{
		if( function ){

			auto s = function( f ) ;

			if( !s.isEmpty() ){

				return s ;
			}
		}

		QString e = f ;

		if( e == "ecryptfs" ){

			e = "ecryptfs-simple" ;
		}

#ifdef Q_OS_WIN
		if( !e.endsWith( ".exe" ) ){

			e += ".exe" ;
		}
#else

#endif
		QString exe ;

		for( const auto& it : utility2::executableSearchPaths() ){

			exe = it + e ;

			if( QFile::exists( exe ) ){

				return exe ;
			}
		}

		return QString() ;
	}

	class translator
	{
	public:
		void setLanguage( const QByteArray& e ) ;
		~translator() ;
	private:
		void clear( void ) ;
		QTranslator * m_translator = nullptr ;
	} ;
}

#endif
