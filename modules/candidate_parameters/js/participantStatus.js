!function(t){function e(s){if(a[s])return a[s].exports;var r=a[s]={exports:{},id:s,loaded:!1};return t[s].call(r.exports,r,r.exports,e),r.loaded=!0,r.exports}var a={};return e.m=t,e.c=a,e.p="",e(0)}([function(t,e){"use strict";Object.defineProperty(e,"__esModule",{value:!0});var a=React.createClass({displayName:"ParticipantStatus",getInitialState:function(){return{Data:[],formData:{},updateResult:null,errorMessage:null,isLoaded:!1,loadedData:0}},componentDidMount:function(){var t=this;$.ajax(this.props.dataURL,{dataType:"json",xhr:function e(){var e=new window.XMLHttpRequest;return e.addEventListener("progress",function(e){t.setState({loadedData:e.loaded})}),e},success:function(e){var a={};a.participantStatus=e.participantStatus,a.participantSuboptions=e.participantSuboptions,a.reasonSpecify=e.reasonSpecify,t.setState({Data:e,formData:a,isLoaded:!0})},error:function(e,a,s){t.setState({error:"An error occurred when loading the form!"})}})},setFormData:function(t,e){var a=this.state.formData,s=this.state.Data.required;"participantStatus"===t&&s.indexOf(e)<0&&(a.participantSuboptions=""),a[t]=e,this.setState({formData:a})},onSubmit:function(t){t.preventDefault()},render:function(){if(!this.state.isLoaded)return void 0!==this.state.error?React.createElement("div",{className:"alert alert-danger text-center"},React.createElement("strong",null,this.state.error)):React.createElement("button",{className:"btn-info has-spinner"},"Loading",React.createElement("span",{className:"glyphicon glyphicon-refresh glyphicon-refresh-animate"}));var t=!0,e=null;loris.userHasPermission("candidate_parameter_edit")&&(t=!1,e=React.createElement(ButtonElement,{label:"Update"}));var a=this.state.Data.required,s=[],r=!1,i=this.state.formData.participantStatus?this.state.formData.participantStatus:this.state.Data.participantStatus;i&&a.indexOf(i)>-1&&(s=this.state.Data.parentIDs[i],r=!0);var n=[];for(var o in this.state.Data.history)if(this.state.Data.history.hasOwnProperty(o)){var c="";for(var l in this.state.Data.history[o])if(this.state.Data.history[o].hasOwnProperty(l)){var p=this.state.Data.history[o][l];if(null!==p)switch(l){case"data_entry_date":c+="[",c+=p,c+="] ";break;case"entry_staff":c+=p,c+=" ";break;case"status":c+=" Status: ",c+=p,c+=" ";break;case"suboption":c+="Details: ",c+=p,c+=" ";break;case"reason_specify":c+="Comments: ",c+=p,c+=" "}}n.push(React.createElement("p",null,c))}var u="",d="alert text-center hide";if(this.state.updateResult)if("success"===this.state.updateResult)d="alert alert-success text-center",u="Update Successful!";else if("error"===this.state.updateResult){var f=this.state.errorMessage;d="alert alert-danger text-center",u=f?f:"Failed to update!"}return React.createElement("div",{class:"row"},React.createElement("div",{className:d,role:"alert",ref:"alert-message"},u),React.createElement(FormElement,{name:"participantStatus",onSubmit:this.handleSubmit,ref:"form",class:"col-md-6"},React.createElement(StaticElement,{label:"PSCID",text:this.state.Data.pscid}),React.createElement(StaticElement,{label:"DCCID",text:this.state.Data.candID}),React.createElement(SelectElement,{label:"Participant Status",name:"participantStatus",options:this.state.Data.statusOptions,value:this.state.formData.participantStatus,onUserInput:this.setFormData,ref:"participantStatus",disabled:t,required:!0}),React.createElement(SelectElement,{label:"Specify Reason",name:"participantSuboptions",options:s,value:this.state.formData.participantSuboptions,onUserInput:this.setFormData,ref:"participantSuboptions",disabled:!r,required:r}),React.createElement(TextareaElement,{label:"Comments",name:"reasonSpecify",value:this.state.formData.reasonSpecify,onUserInput:this.setFormData,ref:"reasonSpecify",disabled:t,required:!1}),e,n))},handleSubmit:function(t){t.preventDefault();var e=this.state.formData,a=this,s=new FormData;for(var r in e)""!==e[r]&&s.append(r,e[r]);s.append("tab",this.props.tabName),s.append("candID",this.state.Data.candID),$.ajax({type:"POST",url:a.props.action,data:s,cache:!1,contentType:!1,processData:!1,success:function(t){a.setState({updateResult:"success"}),a.showAlertMessage()},error:function(t){if(""!==t.responseText){var e=JSON.parse(t.responseText).message;a.setState({updateResult:"error",errorMessage:e}),a.showAlertMessage()}}})},showAlertMessage:function(){var t=this;if(null!==this.refs["alert-message"]){var e=this.refs["alert-message"];$(e).fadeTo(2e3,500).delay(3e3).slideUp(500,function(){t.setState({updateResult:null})})}}}),s=React.createFactory(a);window.ParticipantStatus=a,window.RParticipantStatus=s,e.default=a}]);
//# sourceMappingURL=participantStatus.js.map